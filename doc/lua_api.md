# SM.EXEC binding to Lua
`sm = require("sm")`
## SM.DCB
DCB (Data Control Block) - is a Lua data element linked to data element of any of {`sm.event`, `sm.state`, `sm.tx`, `sm.exec`}. DCB is used to access data blocks within SM.EXEC objects indexed by application key string. DCB keeps track of the current read/write position within its data block.
#### Get DCB
`e1 = sm.new_event(size)`<br>
`eData = e1:get_DCB(key1)`<br>
`s1 = sm.new_state()`<br>
`sData = s1:get_DCB(key2)`<br>
#### Write integer
`eData:write_int(int)`<br>
#### Read integer
`eData:read_int(int)`<br>
#### Allocate array of integers
`eData:allocate_int_array(size)`<br>
#### Read value at pos in array
`i = eData:int1d_get(pos)`<br>
#### Write value at pos in array
`eData:int1d_set(pos)`<br>
#### Allocate 2d array of integers
`eData:allocate_2d_int_array(size)`<br>
#### Read value at (row, col) in 2d array
`i = eData:int2d_get(row, col)`<br>
#### Write value at (row, col) in 2d array
`eData:int2d_set(row, col)`<br>
#### Write string
`eData:write_str(str)`<br>
#### Read string
`str = eData:write_str(str)`<br>
#### Skip n bytes
`eData:skip(n)`<br>
#### Aligb
`eData:align()`<br>
#### Rewind
`eData:rewind()`<br>
#### Size
`#eData`<br>
___
## SM.EXEC
Main Lua wrapper process singleton. Created on init, exposed to Lua as global name "sm_exec".
#### Get exec data block as state_tostring
`str = exec[app_key]`<br>
_Deprecated, use DCB access instead)_
#### Get DCB
`execData = _G.sm_exec:getDCB(key)`<br>
___
## SM.DIRECTORY
Not accessible in Lua other than through stored elements special getters and setters. Implemented for applciations, FSM-s, queues and arrays of states. Getters are supported only for events and states.
#### Register app in the dir
`a1:set_app(key)`<br>
#### Get app from dir
`a2 = sm.get_app(key)`<br>
#### Register fsm in the dir
`f1:dir_set(key)`<br>
#### Get fsm from dir
`f2 = sm.dir_get(key)`<br>
#### Register queue/queue2/pqueue/array
`q:dir_set(key)`<br>
___
## SM.EVENT
The main trolley of SM.EXEC.
#### Create event:
`e = sm.new_event(size)`<br>
#### Set event data:
`e:set(string)`<BR>
_(Deprecated, use DCB access instead)_
#### Get event data:
`str = e:get()`<br>
_(Deprecated, use DCB access instead)_
#### Set event id:
`e:set_id(int)`<br>
#### Get event id:
`e:id() -> int`<br>
#### Set event priority:
`e:set_priority(int, int)`<br>
#### Get event priority:
`[p0, p1] = e:get_priority()`<br>
#### Get DCB
`eventData = e1:get_DCB(key)`<br>
#### Chain events
`e1..e2`<br>
#### Next event in chain
`e3 = e1:next()`<br>
#### Unlink events
`-e1`<br>
#### Event data block size
`#e`<br>
_(Deprecated)_
#### Get event data block as string
`str = e[app_key]`<br>
_Deprecated, use DCB access instead)_
___
## SM.QUEUE
Basic event queue
#### Create queue:
`q = sm.new_queue(queue_size, event_size, sync)`<br>
#### Get top event (w/o extracting)
`e = q:top()`<br>
#### Dequeue event
`e = q:dequeue()`<br>
#### Enqueue event
`q:enqueue(e)`<br>
#### Register queue in the Directory
`q:dir_set(key)`<br>
#### Get a number of events
`#q`<br>
___
## SM.PQUEUE
Priority queue
#### Create priority queue:
`pq = sm.new_pqueue(capacity, sync)`<br>
#### Get top event (w/o extracting)
`e = pq:top()`<br>
#### Dequeue event
`e = pq:dequeue()`<br>
#### Enqueue event
`pq:enqueue(e)`<br>
#### Register queue in the Directory
`pq:dir_set(key)`<br>
#### Get a number of events
`#pq`<br>
___
## SM.QUEUE2
Biproprity queue
`q2 = sm.new_queue2()`<br>
#### Get top event from normal line
`e = q2:get()`<br>
#### Get top event from 'fast' line
`e = q2:get_high()`<br>
#### Enqueue event
`q2:enqueue(e)`<br>
#### Enqueue event in 'fast' line
`q2:enqueue_high(e)`<br>
#### Enqueue event with lock
`q2:lock_enqueue(e)`<br>
#### Enqueue event in 'fast' line with lock
`q2:lock_enqueue_high(e)`<br>
#### Dequeue event
`e = q:dequeue()`<br>
#### Dequeue event with lock
`e = q:lock_dequeue()`<br>
#### Register queue in the Directory
`q2:dir_set(key)`<br>
#### Get a number of events
`#q2`<br>
___
## SM.APP
Applicationfeature(elementarnative application) Lua handler
#### Load dynamic lib (.so) with apps
`handle = sm.load_applib(filename)`<br>
#### Find app in the lib (handle)
`app = sm.lookup(handle, name)`<br>
#### Call application
`app(event, state)`
#### Register app in the dir
`a1:dir_set(key)`<br>
#### Get app from dir
`a2 = sm.dir_get(key)`<br>
___
## SM.FSM
FSM LUA handler
#### Create new state machine
`sm = sm.new_fsm(json, type)`<br>
`json` - state machine description in JSON<br>
`type = {"mealy" | "moore"}`<br>
#### Register fsm in the dir
`f1:dir_set(key)`<br>
#### Get fsm from dir
`f2 = sm.dir_get(key)`<br>
#### Make fsm collectable
`f:free()`<br>
___
## SM.STATE
State of the service containing application context with reference to FSM and specific current state in it.
#### Create new state machine
`s1 = sm.new_state(fsm, data_block_size)`<br>
#### Add event to the trace
`s1:trace_add(e1)`<br>
#### Get top the top event in trace (w/o removing)
`e2 = s1:trace_get()`<br>
#### Set state data:
`s1:set(string)`<BR>
_(Deprecated, use DCB access instead)_
#### Get state data:
`str = s1:get()`<br>
_(Deprecated, use DCB access instead)_
#### Set state key:
`s1:set_key(string)`<BR>
#### Set state key:
`str = s1:get_key()`<BR>
#### Set state id:
`s1:set_id(int)`<br>
#### Get state id:
`i = e:id()`<br>
#### Get state DCB
`stateData = s1:get_DCB(key)`<br>
#### Get state data block as string
`str = s1[app_key]`<br>
_Deprecated, use DCB access instead)_
#### Apply event to state
`s1:apply(e1)`<br>
#### Purge all state information & detach
`s1:purge()`<br>
___
## SM.ARRAY
Hash array of states (applicaion contexts)
#### Create new state array
`a = sm.new_array(stack_size, state_data_block_size)`<br>
#### Get number of states in stack
`#a`<br>
#### Get state by key
`s1 = a:get(key)`<br>
#### Find state by ey
`s1 = a[key]`<br>
#### Release state
`a:release(s1)`<br>
#### Register array in the dir
`f1:dir_set(key)`<br>
___
## SM.TX
Thread-worker descritor (Lua handler for).
#### Create new tx
`tx = sm.new_tx(fsm, queue2, sync, tx_data_block_size)`<br>
`fsm` and `queue2` are given by Directory names
#### Get tx data block as string by key
`str = tx[app_key]`<br>
_Deprecated, use DCB access instead)_
#### Get tx DCB
`txData = tx:get_DCB(key)`<br>
#### Run thread-worker
`tx:run()`<br>
___
