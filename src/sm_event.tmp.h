
/* Lifecycle */
sm_event *sm_event_create(uint32_t data_size);
sm_event *sm_event_ext_create(uint32_t data_size
							  bool disposable_flag, 
						  	  bool hash_key_flag,
						 	  bool priority_flag,
						      sm_queue * home);
void sm_event_free(sm_event *e);
void sm_event_purge(sm_event *e);
void sm_event_park(sm_event *e);
bool sm_event_is_disposable(sm_event *e) {return (bool)e->ctl.disposable_flag;}
/* Chaining */
void sm_event_link(sm_event *e1, sm_event *e2);
sm_event *sm_event_unlink(sm_event *e);
sm_event *sm_event_tail(sm_event *e) {return e->ctl.tailed_flag ? e->next : NULL;}
bool sm_event_is_linked(sm_event *e) {return (bool)e->ctl.tailed_flag;}
/* Id */
size_t sm_event_id(event *e) {return (size_r)e->ctl.id;}
void sm_event_set_id(event *e, size_t id) {e->ctl.id = (uint16_t)id;} // assert value? trunkate and signal? 
#define	SM_EVENT_ID(E) ((E)->ctl->id) //!!
/* Data */
void *sm_event_data(event *e);
#define	SM_EVENT_DATA(E) ((void *)(E) + (E)->ctl.data_offset + 2) //--??
size_t sm_event_data_size(event	*e) {return(size_t)e->data_size;}
/* Home queue */
sm_queue* sm_event_pool_addr(sm_event *e) {
	return (sm_queue *)(e->ctl.pooled_flag ? 
						(void *)e + SM_EVENT_POOL_ADDR_OFFSET(e) : NULL);
}
#define SM_EVENT_POOL_ADDR(E) (sm_queue *)((E)->ctl.pooled_flag ? \
	((void *)(E) + SM_EVENT_POOL_ADDR_OFFSET(E) : NULL) //--??
/* Handle */
void *sm_event_handle(sm_event *e) {return e->ctl.handle_flag ? ((void *)e + SM_EVENT_HANDLE_OFFSET(e) : NULL)}
#define SM_EVENT_HANDLE(E) ((E)->ctl.handle_flag ? ((void *)(E) + SM_EVENT_HANDLE_OFFSET(E) : NULL) //--??
/* Hash key */
sm_hash_key *sm_event_key(sm_event *e) {return (sm_hash_key *)(e->ctl.hash_key_flag ? (void *)e + SM_EVENT_HASH_KEY_OFFSET(e) : NULL);}
#define SM_EVENT_HASH_KEY(E) (sm_hash_key *)((E)->ctl.hash_key_flag ? ((void *)(E) + SM_EVENT_HASH_KEY_OFFSET(E) : NULL) //--??
/* Priority */
long long *sm_event_priority(sm_event *e){(long long *)(e->ctl.priority_flag ? ((void *)e + SM_EVENT_PRIORITY_OFFSET(e) : NULL)}
#define SM_EVENT_PRIORITY(E) (long long *)((E)->ctl.priority_flag ? ((void *)(E) + SM_EVENT_PRIORITY_OFFSET(E) : NULL) //--??

