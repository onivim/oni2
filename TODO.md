Handle 'flushing' cases:

Algorithm:

- bind to ac, a
	- test ac
	- test a + flush
	- 

- bind to ab, a, b:
	- execute 'aa'
	- execute 'ab'
	- execute 'b'

- bind to bc, execute 'ba'
	- case where a is bound (execute) 
	- case where a is not bound (unhandled)
	- case where b is bound (execute)
	- case where b is not bound

- bind to bc, ac, b, and a - execute 'ab'
	- 'a' binding should be executed
	- 'b' binding should be pending

Maybe pivot to storing queued keys instead of
filtered candidate bindings?

Which way do we process bindings in the event of flush?


store key queue
	- flush on no available bindings
		- run longest sub-binding with any match
	- flush on matching binding

- handle keyup:
  - filter out keys when keys up
