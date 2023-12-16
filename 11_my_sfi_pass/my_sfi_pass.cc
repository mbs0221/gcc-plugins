#include "gcc-plugin.h"
#include "plugin-version.h"

// Needed for gimple_opt_pass
#include "basic-block.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "gimple.h"
#include "gimple-iterator.h"
#include "gimple-pretty-print.h"

#include <iostream>
#include <string>
#include <vector>

// We must assert that this plugin is GPL compatible
int plugin_is_GPL_compatible;

namespace {
    // The pass registration code
    struct plugin_info my_sfi_pass_plugin_info = {
        .version = "1.0",
        .help = "This is my sfi pass plugin",
    };

    // The struct for the pass
    const pass_data my_sfi_pass_data = {
        GIMPLE_PASS, /* type */
        "my_sfi_pass", /* name */
        OPTGROUP_NONE, /* optinfo_flags */
        TV_NONE, /* tv_id */
        PROP_gimple_any, /* properties_required */
        0, /* properties_provided */
        0, /* properties_destroyed */
        0, /* todo_flags_start */
        0, /* todo_flags_finish */
    };

    struct my_sfi_pass : gimple_opt_pass {
        my_sfi_pass(gcc::context *ctx)
            : gimple_opt_pass(my_sfi_pass_data, ctx) {}

        virtual unsigned int execute(function *fun) override {
            const char *funcname;
            basic_block bb;

            // get the function name
            funcname = function_name(fun);
            std::cerr << "Running my sfi pass on function " << funcname << "\n";

            // check if the function should be instrumented
            if (!shouldInstrument(funcname)) {
                return 0;
            }

            // iterate over all basic blocks
            FOR_EACH_BB_FN(bb, fun) {
                gimple_stmt_iterator gsi;

                std::cerr << "Basic block " << bb->index << "\n";

                // iterate over all statements in the basic block
                for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
                    gimple *stmt = gsi_stmt(gsi);

                    // Iterate over all operands of the statement
                    for (unsigned i = 0; i < gimple_num_ops(stmt); i++) {
                        tree op = gimple_op(stmt, i);
                        // print the operand
                        std::cerr << "Operand " << i << ": ";
                        print_generic_expr(stderr, op, TDF_NONE);
                        std::cerr << "\n";
                    }

                    // print the statement
                    std::cerr << gimple_code_name[gimple_code(stmt)] << "\n";
                    print_gimple_stmt(stderr, stmt, 0);
                    std::cerr << "\n";
                }
            }

            return 0;
        }

        virtual my_sfi_pass* clone() override {
            return this;
        }

        virtual bool gate(function *fun) override {
            return true;
        }

        // check if the function should be instrumented
        bool shouldInstrument(const std::string &func_name) {
            for (std::string white_func : white_list) {
                if (func_name.find(white_func) != std::string::npos) {
                    return false;
                }
            }
            return true;
        }
    
    private:
        // white function list
        std::vector<std::string> white_list;
    };
}

int plugin_init(struct plugin_name_args *plugin_info, struct plugin_gcc_version *version) {
    struct register_pass_info pass_info;

    pass_info.pass = new my_sfi_pass(g);
    pass_info.reference_pass_name = "optimized";
    pass_info.ref_pass_instance_number = 1;
    pass_info.pos_op = PASS_POS_INSERT_AFTER;

    register_callback(plugin_info->base_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &pass_info);
    return 0;
}