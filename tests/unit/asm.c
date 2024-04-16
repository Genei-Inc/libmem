#include <libmem/libmem.h>
#include "minunit.h"
#include <stdio.h>
#include <memory.h>

#define RTADDR 0x69420

#define X86_32_INST "mov eax, 0x1337"
#define X86_32_INST_BYTES "\xB8\x39\x05\x00\x00"
#define X86_32_INST_SIZE (sizeof(X86_32_INST_BYTES) - 1) /* -1 is necessary to remove the NULL terminator */

#define X86_32_ASM "push ebp; mov ebp, esp; mov eax, 1337; call 0xdeadbeef; mov esp, ebp; pop ebp; ret"
#define X86_32_ASM_BYTES "\x55\x89\xE5\xB8\x39\x05\x00\x00\xE8\xC2\x2A\xA7\xDE\x89\xEC\x5D\xC3"
#define X86_32_ASM_SIZE (sizeof(X86_32_ASM_BYTES) - 1)

#define X86_64_INST "movabs rax, 1337"
#define X86_64_INST_BYTES "\x48\xB8\x39\x05\x00\x00\x00\x00\x00\x00"
#define X86_64_INST_SIZE (sizeof(X86_64_INST_BYTES) - 1)

#define X86_64_ASM "push rbp; mov rbp, rsp; mov eax, [rip + 1337]; call rax; mov rsp, rbp; pop rbp; ret"
#define X86_64_ASM_BYTES "\x55\x48\x89\xE5\x8B\x05\x39\x05\x00\x00\xFF\xD0\x48\x89\xEC\x5D\xC3"
#define X86_64_ASM_SIZE (sizeof(X86_64_ASM_BYTES) - 1)

#define TEST_LM_ASSEMBLE_EX(arch, bits) \
{ \
	lm_byte_t *payload; \
	lm_size_t size; \
	lm_bool_t check_size; \
	lm_bool_t check_content; \
	lm_size_t i; \
\
	size = LM_AssembleEx(arch##_##bits##_ASM, LM_ARCH_##arch, bits, RTADDR, &payload); \
	mu_assert("failed to assemble " #arch "_" #bits " code", size > 0); \
	check_size = size == arch##_##bits##_ASM_SIZE; \
	LM_FreePayload(payload); \
	if (check_size) check_content = memcmp(payload, arch##_##bits##_ASM_BYTES, size) == 0; \
	mu_assert("payload size of " #arch "_" #bits " is incorrect", check_size); \
	\
	printf("<%s_%s PAYLOAD: ", #arch, #bits); \
	for (i = 0; i < size; ++i) printf("%hhx ", payload[i]); \
	printf("> "); \
	fflush(stdout); \
	\
	mu_assert("payload content of " #arch "_" #bits " does not match expected bytes", check_content); \
}

char *test_LM_Assemble(void *arg)
{
	char *code;
	size_t size;
	char *payload;
	lm_inst_t inst;

	/* TODO: Make macro for this */
	if (LM_GetArchitecture() == LM_ARCH_X86) {
		if (LM_GetBits() == 64) {
			code = X86_64_INST;
			size = X86_64_INST_SIZE;
			payload = X86_64_INST_BYTES;
		} else {
			code = X86_32_INST;
			size = X86_32_INST_SIZE;
			payload = X86_32_INST_BYTES;
		}
	} else {
		printf("<WARN: Architecture untested>");
		fflush(stdout);
		return NULL;
	}

	printf("<CODE: %s> <EXPECTED SIZE: %zd> ", code, size);
	fflush(stdout);

	mu_assert("failed to assemble code", LM_Assemble(code, &inst) != 0);

	printf("<SIZE: %zd> ", inst.size);
	fflush(stdout);

	mu_assert("instruction size is incorrect", inst.size == size);
	mu_assert("instruction bytes do not match expected payload", memcmp(inst.bytes, payload, size) == 0);

	return NULL;
}

char *test_LM_AssembleEx(void *arg)
{
	TEST_LM_ASSEMBLE_EX(X86, 64);
	TEST_LM_ASSEMBLE_EX(X86, 32);

	return NULL;
}