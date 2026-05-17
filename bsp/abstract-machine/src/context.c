#include <am.h>
#include <klib.h>
#include <rtthread.h>

// static volatile rt_ubase_t ctx_from; 
// static volatile rt_ubase_t ctx_to; 
// static int from_flag;

struct ctx_switch_info
{
	rt_ubase_t from;
	rt_ubase_t to;
	int from_flag;
};

static Context* ev_handler(Event e, Context *c) {
  switch (e.event) {
		case EVENT_YIELD: 
			rt_thread_t current_thread = rt_thread_self();
			struct ctx_switch_info info = *(struct ctx_switch_info *)current_thread->user_data;
			if (info.from_flag) {
				*(Context**)(info.from) = c;
			}
			c = *(Context**)(info.to);
											break;
    default: printf("Unhandled event ID = %d\n", e.event); assert(0);
  }
  return c;
}

void __am_cte_init() {
  cte_init(ev_handler);
}

void rt_hw_context_switch_to(rt_ubase_t to) {
	struct ctx_switch_info info = {
		.from = 0,
		.to = to,
		.from_flag = 0
	};
	rt_thread_t current_thread = rt_thread_self();
	current_thread->user_data = (uintptr_t)&info;
	// ctx_to = to;
	// from_flag = 0;
	yield();
}

void rt_hw_context_switch(rt_ubase_t from, rt_ubase_t to) {
	struct ctx_switch_info info = {
		.from = from,
		.to = to,
		.from_flag = 1
	};
	rt_thread_t current_thread = rt_thread_self();
	current_thread->user_data = (uintptr_t)&info;
	
	// ctx_from = from;
	// ctx_to = to;
	// from_flag = 1;
	yield();
}

typedef struct {
	void (*texit)(void);
	void (*tentry)(void *);
	void *parameter;
} thread_ctx_t;

void wrapper(void *arg) {
	thread_ctx_t *ctx = (thread_ctx_t *)arg;
	ctx->tentry(ctx->parameter);
	ctx->texit();
	assert(0);
}
void rt_hw_context_switch_interrupt(void *context, rt_ubase_t from, rt_ubase_t to, struct rt_thread *to_thread) {
  assert(0);
}

rt_uint8_t *rt_hw_stack_init(void *tentry, void *parameter, rt_uint8_t *stack_addr, void *texit) {
	uintptr_t stack_top = (uintptr_t)stack_addr & ~(sizeof(uintptr_t) -1);
	thread_ctx_t *ctx;
	uintptr_t stack_new = (stack_top - sizeof(thread_ctx_t)) & ~(sizeof(uintptr_t) -1);
	ctx = (thread_ctx_t *)(stack_top - sizeof(thread_ctx_t));
	ctx->tentry = tentry;
	ctx->parameter = parameter;
	ctx->texit = texit;
	Context *cp = kcontext((Area) { NULL, (rt_uint8_t*) stack_new}, wrapper, ctx);
  return (rt_uint8_t*)cp;
}
