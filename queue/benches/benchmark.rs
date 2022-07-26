use std::sync::Arc;
use std::time::Duration;
use criterion::{black_box, criterion_group, criterion_main, Criterion};
use queue::{lock_based_queue,lock_free_queue};

fn lock_free_test(thread: usize, size: usize) {
    let mut handles = Vec::new();
    let queue = Arc::new(lock_free_queue::LockFreeQueue::new());
    
    for _ in 0..thread {
        let queue = Arc::clone(&queue);
        let handle = std::thread::spawn(move|| {
            for _ in 0..size {
                queue.enqueue(black_box(1));
            }

            for _ in 0..size {
                queue.dequeue();
            }
        });
        handles.push(handle);
    }

    for handle in handles {
        handle.join().unwrap();
    }
}

fn lock_based_test(thread: usize, size: usize) {
    let mut handles = Vec::new();
    let queue = Arc::new(lock_based_queue::LockBasedQueue::new());
    
    for _ in 0..thread {
        let queue = Arc::clone(&queue);
        let handle = std::thread::spawn(move || {
            for _ in 0..size {
                queue.enqueue(black_box(1));
            }

            for _ in 0..size {
                queue.dequeue();
            }
        });
        handles.push(handle);
    }

    for handle in handles {
        handle.join().unwrap();
    }
}

fn bench_lock_free_queue(c: &mut Criterion) {
    c.bench_function("lock_free_queue", |b| {
        b.iter(|| lock_free_test(black_box(8), black_box(100000)))
    });
}

fn bench_lock_based_queue(c: &mut Criterion) {
    c.bench_function("lock_based_queue", |b| {
        b.iter(|| lock_based_test(black_box(8), black_box(100000)))
    });
}

criterion_group!{
    name = benches;
    config = Criterion::default().measurement_time(Duration::new(30, 0));
    targets = bench_lock_free_queue, bench_lock_based_queue
}

criterion_main!(benches);