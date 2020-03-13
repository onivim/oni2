Handle 'flushing' cases:


- bind to bc, execute 'ba'
	- case where a is bound (execute) 
	- case where a is not bound (unhandled)
	- case where b is bound (execute)
	- case where b is not bound

Maybe pivot to storing queued keys instead of
filtered candidate bindings?

store key queue
	- flush on no available bindings
		- run longest sub-binding with any match
	- flush on matching binding

- handle keyup:
  - filter out keys when keys up
