#pragma once

// https://github.com/torvalds/linux/blob/ab63e725b49c80f941446327d79ba5b68593bf5a/tools/virtio/linux/kernel.h#L114-L119
#define UTHREAD_LIKELY(x) __builtin_expect((x), 1)
#define UTHREAD_UNLIKELY(x) __builtin_expect((x), 0)
