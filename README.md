# cache_traits

References:
* [Robust Method to Determine Cache and TLB Characteristics](https://etd.ohiolink.edu/acprod/odb_etd/ws/send_file/send?accession=osu1308256764&disposition=inline)
* [Calibrator](https://github.com/magsilva/calibrator/tree/master)

## Notes

### Current state

So far I only managed to do the first part: finding cache associativity cache size.
On my MacBook Pro with an M2 Max chip I got the following results:
```
Optimized: false
Max data: 67108864
	8:56	16:59	32:52	64:53	128:59	256:60	512:33	1024:18	2048:11	4096:12	8192:9	16384:13	32768:12	65536:10	131072:10	262144:9	524288:9	1048576:9	2097152:9	4194304:10	8388608:9
Large stride: 16384, max spots that fit into set: 12
Cache associativity: 12
Cache size (bytes): 1572864
Cache size (KB): 1536
Cache size (MB): 1.5
```

Cache associativity fluctuates in the 11-12 range, but cache size fluctuates quite a bit
depending on which stride the last jump has happened.

### `maxData`

I picked it manually as the biggest number that didn't cause and immediate crash on array creation.

```
maxData = 1 << 26
```