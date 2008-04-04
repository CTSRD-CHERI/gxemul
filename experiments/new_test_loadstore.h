
#define AAA

struct cpu {
	int pc;
#ifdef AAA
	unsigned char *table0[1048576];		/*  [1048576];  */
#else
	unsigned char **table0[1024];
#endif
};

struct ic {
	int dummy;
	int *arg1;
	int arg2;
	int *arg3;
};

void x(struct cpu *cpu, struct ic *ic);
void y(struct cpu *cpu, struct ic *ic);

