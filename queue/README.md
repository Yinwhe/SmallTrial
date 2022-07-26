Lock-free queue implementation on Rust.
Reference: [Implementing Lock-Free Queues](https://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.53.8674&rep=rep1&type=pdf)

## Quick Start
```rust
fn main() {
    let q = LockFreeQueue::new();

    q.enqueue(123);

    assert_eq!(q.dequeue(), Some(123));
    assert_eq!(q.dequeue(), None);
}
```

## Benchmark
Benchmark between Lock-Free Queue and Lock-Based Queue:

| (Thread, Size) | Queue      | Total Time |
| -------------- | ---------- | ---------- |
| (1, 100000)    | Lock Free  | 7.1454 ms  |
|                | Lock Based | 8.5447 ms  |
| (2, 100000)    | Lock Free  | 22.222 ms  |
|                | Lock Based | 42.024 ms  |
| (4, 100000)    | Lock Free  | 73.086 ms  |
|                | Lock Based | 116.89 ms  |
| (8, 100000)    | Lock Free  | 218.71 ms  |
|                | Lock Based | 289.20 ms  |

