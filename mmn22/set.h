typedef struct set set;

void read_set(set *s, int arr [], int n);

void print_set(set *s);

void union_set (set *s1, set *s2, set *target);

void intersect_set(set *s1, set *s2, set *target);

void sub_set(set *s1, set *s2, set *target);

void symdiff_set(set *s1, set *s2, set *target);

int isEmpty(set *s);

void emptySet(set *s);

void set_bit_on(set *s, int index);

int is_bit_on(set *s, int index);

set* create_set();

void free_set(set* set);
