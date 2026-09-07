## Building Blocks for LowLatencyCPP

This repo is where you can find the right tool for the trade.
All about LowLatencyCPP header only code which one can use based on their needs.

Currently we have

## Allocator_Lab

where we compare 3 allocators to our general purpose new and delete

---

In future we may have low latency queues. other things like multi threading locks etc..
Quant related so we may do
Network related stuff ? maybe benchmark a kernel bypass maybe ?
Template Meta progamming. arguments forwarding.

try implementing vector myself. unique ptr etc.. for learning. linked list with smart pointers
copy vs move
restrict keyword -> pointer not overlapping

vitual indexed , physically mapped tables. 

-> pointer deref -> tlb -> cache

L1 cache optimziation by using virutally indexed. See gfg article


write unique pointer.
Write linked list with smart pointers


doubly linked list -> make cache 

LRU cahce with doubly linked list + hash map 

add smart pointer

PERF tool -> a cache miss is 3ns

Logging framework. -> fast

NETWORKS -

TCP -> 3 way -> why ?

what is TCP, UDP, congestion control, why, what .

kernel bypass.

SPSC queues. producer consumer queues
templates meta programming -> simple needed
argument forwarding.
