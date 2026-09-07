# RESUME POINTS

something along the lines of 

Building Blocks for Low-Latency Applications in modern C++ — Self Project

• Realized inefficiencies in code design arising due to locks/mutexes, dynamic memory allocation, locality neglect and pointer-aliasing

• Implemented thread library with core affinity setting function and perfect forwarding semantics to eliminate runtime uncertainty

• Designed a cache-friendly memory pool using pre-allocated blocks, achieving ~1.5x speedup in allocation/insertion of 100K objects (<20ns/alloc), and >1.3x speedup in random index deletion (1000 live objects over 10K ops), compared to operations on vector

• Built a logging framework which offloads slow string I/O using a non-blocking asynchronous background thread, achieving <300ns avg latency in queueing a log message to an SPSC lock-free queue, which is periodically flushed to a file
