/*
* By shajianfeng 
*
*/
#ifndef LINE_CONFIG_H
#define LINE_CONFIG_H

typedef struct ks_directive_t ks_directive_t;
typedef struct cmd_parms_struct cmd_parms;
typedef struct command_struct command_rec;
typedef struct module_struct module;

/**
 * How the directives arguments should be parsed.
 * @remark Note that for all of these except RAW_ARGS, the config routine is
 *      passed a freshly allocated string which can be modified or stored
 *      or whatever...
 */
enum cmd_how {
	RAW_ARGS,           /**< cmd_func parses command line itself */
	TAKE1,              /**< one argument only */
	TAKE2,              /**< two arguments only */
	ITERATE,            /**< one argument, occuring multiple times
			 * (e.g., IndexIgnore)
			 */
	ITERATE2,           /**< two arguments, 2nd occurs multiple times
			 * (e.g., AddIcon)
			 */
	FLAG,               /**< One of 'On' or 'Off' */
	NO_ARGS,            /**< No args at all, e.g. &lt;/Directory&gt; */
	TAKE12,             /**< one or two arguments */
	TAKE3,              /**< three arguments only */
	TAKE23,             /**< two or three arguments */
	TAKE123,            /**< one, two or three arguments */
	TAKE13,             /**< one or three arguments */
	TAKE_ARGV           /**< an argc and argv are passed */
};


/**
 * This structure is passed to a command which is being invoked,
 * to carry a large variety of miscellaneous data which is all of
 * use to *somebody*...
 */
struct cmd_parms_struct {
	/** Argument to command from cmd_table */
	void *info;

	/** Config file structure. */
	ks_configfile_t *config_file;
	/** the directive specifying this command */
	ks_directive_t *directive;

	/** Pool to allocate new storage in */
	ks_pool_t *pool;
	/** Pool for scratch memory; persists during configuration, but
	*  wiped before the first request is served...  */
	ks_pool_t *temp_pool;
	/** If configuring for a directory, pathname of that directory.
	*  NOPE!  That's what it meant previous to the existence of &lt;Files&gt;,
	* &lt;Location&gt; and regex matching.  Now the only usefulness that can be
	* derived from this field is whether a command is being called in a
	* server context (path == NULL) or being called in a dir context
	* (path != NULL).  */
	char *path;
	/** configuration command */
	const command_rec *cmd;

	/** per_dir_config vector passed to handle_command */
	struct ap_conf_vector_t *context;

};
/**
 * @brief Structure used to build the config tree.
 *
 * The config tree only stores
 * the directives that will be active in the running server.  Directives
 * that contain other directions, such as &lt;Directory ...&gt; cause a sub-level
 * to be created, where the included directives are stored.  The closing
 * directive (&lt;/Directory&gt;) is not stored in the tree.
 */
struct ks_directive_t {
	/** The current directive */
	const char *directive;
	/** The arguments for the current directive, stored as a space
	*  separated list */
	const char *args;
	/** The next directive node in the tree */
	struct ks_directive_t *next;
	/** The first child node of this directive */
	struct ks_directive_t *first_child;
	/** The parent node of this directive */
	struct ks_directive_t *parent;

	/** directive's module can store add'l data here */
	void *data;

	/* ### these may go away in the future, but are needed for now */
	/** The name of the file this directive was found in */
	const char *filename;
	/** The line number the directive was on */
	int line_num;

	/** A short-cut towards the last directive node in the tree.
	*  The value may not always be up-to-date but it always points to
	*  somewhere in the tree, nearer to the tail.
	*  This value is only set in the first node
	*/
	struct ks_directive_t *last;
};

/**
 * All the types of functions that can be used in directives
 * @internal
 */
typedef union {
    /** function to call for a no-args */
    const char *(*no_args) (cmd_parms *parms, void *mconfig);
    /** function to call for a raw-args */
    const char *(*raw_args) (cmd_parms *parms, void *mconfig,
                             const char *args);
    /** function to call for a argv/argc */
    const char *(*take_argv) (cmd_parms *parms, void *mconfig,
                             int argc, char *const argv[]);
    /** function to call for a take1 */
    const char *(*take1) (cmd_parms *parms, void *mconfig, const char *w);
    /** function to call for a take2 */
    const char *(*take2) (cmd_parms *parms, void *mconfig, const char *w,
                          const char *w2);
    /** function to call for a take3 */
    const char *(*take3) (cmd_parms *parms, void *mconfig, const char *w,
                          const char *w2, const char *w3);
    /** function to call for a flag */
    const char *(*flag) (cmd_parms *parms, void *mconfig, int on);
} cmd_func;

/** This configuration directive does not take any arguments */
# define KS_NO_ARGS     func.no_args
/** This configuration directive will handle its own parsing of arguments*/
# define KS_RAW_ARGS    func.raw_args
/** This configuration directive will handle its own parsing of arguments*/
# define KS_TAKE_ARGV   func.take_argv
/** This configuration directive takes 1 argument*/
# define KS_TAKE1       func.take1
/** This configuration directive takes 2 arguments */
# define KS_TAKE2       func.take2
/** This configuration directive takes 3 arguments */
# define KS_TAKE3       func.take3
/** This configuration directive takes a flag (on/off) as a argument*/
# define KS_FLAG        func.flag

/** mechanism for declaring a directive with no arguments */
# define KS_INIT_NO_ARGS(directive, func, mconfig, where, help) \
    { directive, { .no_args=func }, mconfig, where, RAW_ARGS, help }
/** mechanism for declaring a directive with raw argument parsing */
# define KS_INIT_RAW_ARGS(directive, func, mconfig, where, help) \
    { directive, { .raw_args=func }, mconfig, where, RAW_ARGS, help }
/** mechanism for declaring a directive with raw argument parsing */
# define KS_INIT_TAKE_ARGV(directive, func, mconfig, where, help) \
    { directive, { .take_argv=func }, mconfig, where, TAKE_ARGV, help }
/** mechanism for declaring a directive which takes 1 argument */
# define KS_INIT_TAKE1(directive, func, mconfig, where, help) \
    { directive, { .take1=func }, mconfig, where, TAKE1, help }
/** mechanism for declaring a directive which takes multiple arguments */
# define KS_INIT_ITERATE(directive, func, mconfig, where, help) \
    { directive, { .take1=func }, mconfig, where, ITERATE, help }
/** mechanism for declaring a directive which takes 2 arguments */
# define KS_INIT_TAKE2(directive, func, mconfig, where, help) \
    { directive, { .take2=func }, mconfig, where, TAKE2, help }
/** mechanism for declaring a directive which takes 1 or 2 arguments */
# define KS_INIT_TAKE12(directive, func, mconfig, where, help) \
    { directive, { .take2=func }, mconfig, where, TAKE12, help }
/** mechanism for declaring a directive which takes multiple 2 arguments */
# define KS_INIT_ITERATE2(directive, func, mconfig, where, help) \
    { directive, { .take2=func }, mconfig, where, ITERATE2, help }
/** mechanism for declaring a directive which takes 1 or 3 arguments */
# define KS_INIT_TAKE13(directive, func, mconfig, where, help) \
    { directive, { .take3=func }, mconfig, where, TAKE13, help }
/** mechanism for declaring a directive which takes 2 or 3 arguments */
# define KS_INIT_TAKE23(directive, func, mconfig, where, help) \
    { directive, { .take3=func }, mconfig, where, TAKE23, help }
/** mechanism for declaring a directive which takes 1 to 3 arguments */
# define KS_INIT_TAKE123(directive, func, mconfig, where, help) \
    { directive, { .take3=func }, mconfig, where, TAKE123, help }
/** mechanism for declaring a directive which takes 3 arguments */
# define KS_INIT_TAKE3(directive, func, mconfig, where, help) \
    { directive, { .take3=func }, mconfig, where, TAKE3, help }
/** mechanism for declaring a directive which takes a flag (on/off) argument */
# define KS_INIT_FLAG(directive, func, mconfig, where, help) \
    { directive, { .flag=func }, mconfig, where, FLAG, help }

struct command_struct{
	/* name of this command */
	const char *name;
	/*The function te be called when this directive is parsed*/
	cmd_func func;
	/*Extra data,for functions which implemente multiple commands */
	void *cmd_data;
	/* What overrides need to be allowed to enable this command.*/
	int req_override;
	/* What the command expects as arguments */
	enum cmd_how args_how;
	/* * 'usage' message, in case of syntax errors*/
	const char *errmsg;
};

struct module_struct{
	struct list_head anchor;
	const char *name;
}

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/**
 * Add a node to the configuration tree.
 * @param parent The current parent node.  If the added node is a first_child,
                 then this is changed to the current node
 * @param current The current node
 * @param toadd The node to add to the tree
 * @param child Is the node to add a child node
 * @return the added node
 */
ks_directive_t *ks_add_node(ks_directive_t **parent, ks_directive_t *current,
                            ks_directive_t *toadd, int child);

ks_int_t ks_read_config(ks_pool_t *ptemp,const char* filename,ks_directive_t **conftree);

#ifdef __cplusplus
}
#endif /*__cplusplus*/
#endif /*LINE_CONFIG_H*/
