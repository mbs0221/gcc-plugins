#include <iostream>

// This is the first gcc header to be included
#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree-pass.h"
#include "gimple-pretty-print.h"
#include "tree.h"
#include "gimple.h"
#include "gimple-iterator.h"
#include "gimple-ssa.h"
#include "tree-phinodes.h"
#include "tree-ssa-operands.h"
#include "tree-cfg.h"
#include "tree-ssa.h"
#include "ssa-iterators.h"

// Needed for gimple_opt_pass
#include "tree-pass.h"
#include "context.h"

// We must assert that this plugin is GPL compatible
int plugin_is_GPL_compatible;

static struct plugin_info my_gcc_plugin_info = { "1.0", "This is a very simple plugin" };


namespace
{
    const pass_data fork_pass_data = 
    {
        GIMPLE_PASS,
        "fork_pass",         /* name */
        OPTGROUP_NONE,          /* optinfo_flags */
        TV_NONE,                /* tv_id */
        PROP_gimple_any,        /* properties_required */
        0,                      /* properties_provided */
        0,                      /* properties_destroyed */
        0,                      /* todo_flags_start */
        0                       /* todo_flags_finish */
    };

    struct fork_pass : gimple_opt_pass
    {
        fork_pass(gcc::context *ctx)
            : gimple_opt_pass(fork_pass_data, ctx)
        {
        }

        void insert_vmpl_enter(gimple *stmt, basic_block bb)
        {
            // Create a basic block for the if condition when stmt return 0
            // pid_t pid = fork();
            // if (pid == 0) {
            //     vmpl_enter(1, 0);
            //     if (vmpl_enter(1, 0) != 0) {
            //         exit(EXIT_FAILURE);
            //     }
            // }
            // Let's write the above code in GIMPLE first and then convert it to C code
            // Please generate a gcc-plugin for this requirement for the fork() function call


            // Create the if condition for stmt return 0
            tree lhs = gimple_call_lhs(stmt);
            tree zero = build_int_cst(integer_type_node, 0);
            gcond *if_stmt = gimple_build_cond(EQ_EXPR, lhs, zero, NULL_TREE, NULL_TREE);

            // Create the vmpl_enter function call
            tree vmpl_enter_fn = build_fn_decl("vmpl_enter", build_function_type(integer_type_node, tree_cons(NULL_TREE, integer_type_node, tree_cons(NULL_TREE, ptr_type_node, NULL_TREE))));
            tree vmpl_enter_arg1 = build_int_cst(integer_type_node, 1);
            tree vmpl_enter_arg2 = build_int_cst(ptr_type_node, 0);
            gcall *vmpl_enter_call = gimple_build_call(vmpl_enter_fn, 2, vmpl_enter_arg1, vmpl_enter_arg2);

            // Create the condition to check if vmpl_enter returns 0
            tree lhs_vmpl = gimple_call_lhs(vmpl_enter_call);
            gcond *if_stmt_vmpl = gimple_build_cond(NE_EXPR, lhs_vmpl, zero, NULL_TREE, NULL_TREE);

            // Create the exit(EXIT_FAILURE) call
            tree exit_fn = build_fn_decl("exit", build_function_type(void_type_node, tree_cons(NULL_TREE, integer_type_node, NULL_TREE)));
            tree exit_arg = build_int_cst(integer_type_node, EXIT_FAILURE);
            gcall *exit_call = gimple_build_call(exit_fn, 1, exit_arg);

            // Create a new basic block for the if condition
            edge e;
            edge_iterator ei;
            basic_block new_bb = create_empty_bb(bb);
            FOR_EACH_EDGE(e, ei, bb->succs)
            {
                redirect_edge_pred(e, new_bb);
            }
            make_edge(bb, new_bb, EDGE_FALLTHRU);
            make_edge(new_bb, EXIT_BLOCK_PTR_FOR_FN(cfun), 0);

            // Insert the if statement into the new basic block
            gimple_stmt_iterator gsi = gsi_after_labels(new_bb);
            gsi_insert_after(&gsi, if_stmt, GSI_NEW_STMT);

            // Insert the vmpl_enter call into the new basic block
            gsi_insert_after(&gsi, vmpl_enter_call, GSI_NEW_STMT);

            // Insert the if statement for vmpl_enter into the new basic block
            gsi_insert_after(&gsi, if_stmt_vmpl, GSI_NEW_STMT);

            // Insert the exit call into the new basic block
            gsi_insert_after(&gsi, exit_call, GSI_NEW_STMT);
        }

        // After each fork function call, insert a vmpl_enter function call when the return value of the fork is 0, then on vmpl_enter return non zero, call exit(EXIT_FAILURE) to exit pthread
        virtual unsigned int execute(function *fun) override
        {
            std::cerr << "Running my first pass, OMG\n";

            basic_block bb;
            gimple_stmt_iterator gsi;

            // Iterate over all basic blocks in the function
            FOR_EACH_BB_FN(bb, fun)
            {
                // Iterate over all statements in the basic block
                for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi))
                {
                    gimple *stmt = gsi_stmt(gsi);

                    if (is_gimple_call(stmt))
                    {
                        tree fn_decl = gimple_call_fndecl(stmt);
                        if (fn_decl && DECL_BUILT_IN_CLASS(fn_decl) == BUILT_IN_NORMAL && strcmp(IDENTIFIER_POINTER(DECL_NAME(fn_decl)), "fork") == 0)
                        {
                            std::cerr << "Found a fork call\n";
                            insert_vmpl_enter(stmt, bb);
                        }
                    }
                }
            }

            FOR_EACH_BB_FN(bb, fun)
            {
                fprintf(stderr, "Basic Block %d\n", bb->index);
                gimple_bb_info *bb_info = &bb->il.gimple;
                print_gimple_seq(stderr, bb_info->seq, 0, static_cast<dump_flags_t>(0));
                fprintf(stderr, "\n");
            }

            return 0;
        }

        virtual fork_pass* clone() override
        {
            // We do not clone ourselves
            return this;
        }
    };
}

int plugin_init (struct plugin_name_args *plugin_info,
		struct plugin_gcc_version *version)
{
	// We check the current gcc loading this plugin against the gcc we used to
	// created this plugin
	if (!plugin_default_version_check (version, &gcc_version))
    {
        std::cerr << "This GCC plugin is for version " << GCCPLUGIN_VERSION_MAJOR << "." << GCCPLUGIN_VERSION_MINOR << "\n";
		return 1;
    }

    register_callback(plugin_info->base_name,
            /* event */ PLUGIN_INFO,
            /* callback */ NULL, /* user_data */ &my_gcc_plugin_info);

    struct register_pass_info pass_info;

    // Register the phase right before cfg
    pass_info.pass = new fork_pass(g);
    pass_info.reference_pass_name = "cfg";
    pass_info.ref_pass_instance_number = 1;
    pass_info.pos_op = PASS_POS_INSERT_AFTER;

    register_callback (plugin_info->base_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &pass_info);

    return 0;
}