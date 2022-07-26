use std::ptr;
use std::sync::atomic::{AtomicPtr, Ordering};

struct Node<T: Send> {
    value: Option<T>,
    next: AtomicPtr<Node<T>>,
}

impl<T: Send> Node<T> {
    fn new(value: T) -> Self {
        Node {
            value: Some(value),
            next: AtomicPtr::new(ptr::null_mut()),
        }
    }

    fn dummy() -> Self {
        Node {
            value: None,
            next: AtomicPtr::new(ptr::null_mut()),
        }
    }
}

pub struct LockFreeQueue<T: Send> {
    head: AtomicPtr<Node<T>>,
    tail: AtomicPtr<Node<T>>,
}

impl<T: Send> Drop for LockFreeQueue<T> {
    fn drop(&mut self) {
        let mut cur = self.head.load(Ordering::Relaxed);

        while cur != ptr::null_mut() {
            let next = unsafe { (*cur).next.load(Ordering::Relaxed) };

            unsafe { cur.drop_in_place() }

            cur = next;
        }
        std::mem::drop(self)
    }
}

impl<T: Send> LockFreeQueue<T> {
    pub fn new() -> Self {
        let p = Box::leak(Box::new(Node::dummy()));

        let res = LockFreeQueue {
            head: AtomicPtr::new(ptr::null_mut()),
            tail: AtomicPtr::new(ptr::null_mut()),
        };
        res.head.store(p, Ordering::Relaxed);
        res.tail.store(p, Ordering::Relaxed);

        return res;
    }

    pub fn enqueue(&self, value: T) {
        let q = Box::leak(Box::new(Node::new(value)));
        let mut p = self.tail.load(Ordering::Acquire);

        loop {
            p = self.tail.load(Ordering::Acquire);

            let succ = unsafe {
                (*p).next
                    .compare_exchange(ptr::null_mut(), q, Ordering::Release, Ordering::Relaxed)
            }
            .is_ok();

            if !succ {
                self.tail.compare_exchange(
                    p,
                    unsafe { (*p).next.load(Ordering::Acquire) },
                    Ordering::Release,
                    Ordering::Relaxed,
                );
            } else {
                break;
            }
        }

        self.tail
            .compare_exchange(p, q, Ordering::Release, Ordering::Relaxed);
    }

    pub fn dequeue(&self) -> Option<T> {
        let mut p = self.head.load(Ordering::Acquire);
        loop {
            p = self.head.load(Ordering::Acquire);

            if unsafe { (*p).next.load(Ordering::Acquire) == ptr::null_mut() } {
                return None;
            }

            if self
                .head
                .compare_exchange(
                    p,
                    unsafe { (*p).next.load(Ordering::Acquire) },
                    Ordering::Release,
                    Ordering::Relaxed,
                )
                .is_ok()
            {
                break;
            }
        }

        let res = unsafe { (*(*p).next.load(Ordering::Acquire)).value.take() };
        unsafe { p.drop_in_place() };

        return res;
    }
}

#[test]
fn test() {
    use std::sync::{Arc, Barrier};
    use std::thread;

    let lfq = Arc::new(LockFreeQueue::new());
    let lfq_clone = Arc::clone(&lfq);

    let barrier = Arc::new(Barrier::new(2));
    let barrier_clone = Arc::clone(&barrier);

    let handle = thread::spawn(move || {
        barrier_clone.wait();
        for i in 0..100 {
            // println!("Inserting {}", i);
            lfq_clone.enqueue(i);
        }
        println!("done1");
    });

    barrier.wait();
    for i in 100..200 {
        // println!("Inserting {}", i);
        lfq.enqueue(i);
    }
    println!("done2");

    handle.join().unwrap();

    while let Some(v) = lfq.dequeue() {
        println!("{}", v);
    }
}
