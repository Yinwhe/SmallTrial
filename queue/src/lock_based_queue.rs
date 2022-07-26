use std::collections::LinkedList;
use std::sync::Mutex;

pub struct LockBasedQueue<T: Send> {
    inner: Mutex<LinkedList<T>>
}

impl<T: Send> LockBasedQueue<T> {
   pub fn new() -> Self {
        LockBasedQueue {
            inner: Mutex::new(LinkedList::new())
        }
    }

    pub fn enqueue(&self, value: T) {
        self.inner.lock().unwrap().push_back(value);
    }

    pub fn dequeue(&self) -> Option<T> {
        self.inner.lock().unwrap().pop_front()
    }
}


#[test]
fn test() {
    use std::sync::{Arc, Barrier};
    use std::thread;

    let lfq = Arc::new(LockBasedQueue::new());
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
