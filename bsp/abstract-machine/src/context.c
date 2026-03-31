#include <am.h>
#include <klib.h>
#include <rtthread.h>

static volatile rt_ubase_t ctx_from; 
static volatile rt_ubase_t ctx_to; 
static int from_flag;

static Context* ev_handler(Event e, Context *c) {
  switch (e.event) {
		case EVENT_YIELD: 
			if (from_flag) *(Context**)ctx_from = c;
			c = *(Context**)ctx_to;  
											break;
    default: printf("Unhandled event ID = %d\n", e.event); assert(0);
  }
  return c;
}

void __am_cte_init() {
  cte_init(ev_handler);
}

void rt_hw_context_switch_to(rt_ubase_t to) {
	ctx_to = to;
	from_flag = 0;
	yield();
  assert(0);
}

void rt_hw_context_switch(rt_ubase_t from, rt_ubase_t to) {
	ctx_from = from;
	ctx_to = to;
	from_flag = 1;
	yield();
  assert(0);
}

void rt_hw_context_switch_interrupt(void *context, rt_ubase_t from, rt_ubase_t to, struct rt_thread *to_thread) {
  assert(0);
}

rt_uint8_t *rt_hw_stack_init(void *tentry, void *parameter, rt_uint8_t *stack_addr, void *texit) {
	uintptr_t stack_top = (uintptr_t)stack_addr & ~(sizeof(uintptr_t) -1);
	Context *cp = kcontext((Area) { NULL, (rt_uint8_t*) stack_top}, tentry, parameter);
	cp->gpr[1] = (uintptr_t)texit;	
  return (rt_uint8_t*)cp;
}
