#ifndef ZZGO_UCT_INTERNAL_H
#define ZZGO_UCT_INTERNAL_H

#include <signal.h> // sig_atomic_t

#include "debug.h"
#include "move.h"
#include "ownermap.h"
#include "playout.h"
#include "stats.h"

struct tree;
struct tree_node;
struct uct_policy;
struct uct_prior;

/* Internal UCT structures */


/* Internal engine state. */
struct uct {
	int debug_level;
	int games, gamelen;
	float resign_ratio;
	float loss_threshold;
	double best2_ratio, bestr_ratio;
	bool pass_all_alive;
	bool territory_scoring;
	int expand_p;
	bool playout_amaf, playout_amaf_nakade;
	bool amaf_prior;
	int playout_amaf_cutoff;
	int dumpthres;
	int force_seed;
	bool no_book;
	bool fast_alloc;
	unsigned long max_tree_size;
	int mercymin;

	int threads;
	enum uct_thread_model {
		TM_ROOT, /* Root parallelization. */
		TM_TREE, /* Tree parallelization w/o virtual loss. */
		TM_TREEVL, /* Tree parallelization with virtual loss. */
	} thread_model;
	bool parallel_tree;
	bool virtual_loss;
	bool pondering_opt; /* User wants pondering */
	bool pondering; /* Actually pondering now */

	int fuseki_end;
	int yose_start;

	int dynkomi;
	int dynkomi_mask;
	int handicap_value;

	float val_scale;
	int val_points;
	bool val_extra;

	int random_policy_chance;
	int local_tree;
	int tenuki_d;
	float local_tree_aging;
	bool local_tree_allseq;

	char *banner;

	struct uct_policy *policy;
	struct uct_policy *random_policy;
	struct playout_policy *playout;
	struct uct_prior *prior;

	/* Used within frame of single genmove. */
	struct board_ownermap ownermap;

	/* Game state - maintained by setup_state(), reset_state(). */
	struct tree *t;
};

#define UDEBUGL(n) DEBUGL_(u->debug_level, n)

extern volatile sig_atomic_t uct_halt;
extern __thread int thread_id;

bool uct_pass_is_safe(struct uct *u, struct board *b, enum stone color, bool pass_all_alive);


/* This is the state used for descending the tree; we use this wrapper
 * structure in order to be able to easily descend in multiple trees
 * in parallel (e.g. main tree and local tree) or compute cummulative
 * "path value" throughout the tree descent. */
struct uct_descent {
	/* Active tree nodes: */
	struct tree_node *node; /* Main tree. */
	struct tree_node *lnode; /* Local tree. */
	/* Value of main tree node (with all value factors, but unbiased
	 * - without exploration factor), from black's perspective. */
	struct move_stats value;
};


typedef struct tree_node *(*uctp_choose)(struct uct_policy *p, struct tree_node *node, struct board *b, enum stone color, coord_t exclude);
typedef float (*uctp_evaluate)(struct uct_policy *p, struct tree *tree, struct uct_descent *descent, int parity);
typedef void (*uctp_descend)(struct uct_policy *p, struct tree *tree, struct uct_descent *descent, int parity, bool allow_pass);
typedef void (*uctp_winner)(struct uct_policy *p, struct tree *tree, struct uct_descent *descent);
typedef void (*uctp_prior)(struct uct_policy *p, struct tree *tree, struct tree_node *node, struct board *b, enum stone color, int parity);
typedef void (*uctp_update)(struct uct_policy *p, struct tree *tree, struct tree_node *node, enum stone node_color, enum stone player_color, struct playout_amafmap *amaf, float result);

struct uct_policy {
	struct uct *uct;
	uctp_choose choose;
	uctp_winner winner;
	uctp_evaluate evaluate;
	uctp_descend descend;
	uctp_update update;
	uctp_prior prior;
	bool wants_amaf;
	void *data;
};

#endif
