add_rules('mode.release', 'mode.debug')
set_languages('c11', 'cxx17')

target("libUriParser")
    set_kind("static")
    add_files(
        "3rdparty/uriparser/src/UriCommon.c",
        "3rdparty/uriparser/src/UriCompare.c",
        "3rdparty/uriparser/src/UriEscape.c",
        "3rdparty/uriparser/src/UriFile.c",
        "3rdparty/uriparser/src/UriIp4.c",
        "3rdparty/uriparser/src/UriIp4Base.c",
        "3rdparty/uriparser/src/UriNormalize.c",
        "3rdparty/uriparser/src/UriNormalizeBase.c",
        "3rdparty/uriparser/src/UriParse.c",
        "3rdparty/uriparser/src/UriParseBase.c",
        "3rdparty/uriparser/src/UriQuery.c",
        "3rdparty/uriparser/src/UriRecompose.c",
        "3rdparty/uriparser/src/UriResolve.c",
        "3rdparty/uriparser/src/UriShorten.c",
        "3rdparty/uriparser/src/UriMemory.c"
    )
    add_defines("URIPARSER_BUILD_CHAR", "URI_STATIC_BUILD", {public = true})
    add_includedirs("3rdparty/uriparser/include", {public = true})
target_end()

target("libDaScript")
    set_kind("static")
    add_deps("libUriParser")
    add_includedirs("include", {public = true})
    add_includedirs("3rdparty/fmt/include", {public = true})
    
    add_defines("DAS_INSTALL_BINDIR=\"bin\"")
    
    -- parser
    add_files(
        "src/parser/parser_impl.cpp"
    )

    -- parser/generated
    add_files(
        "src/parser/ds_parser.cpp",
        "src/parser/ds_lexer.cpp"
    )

    -- ast
    add_files(
        "src/ast/ast.cpp",
        "src/ast/ast_tls.cpp",
        "src/ast/ast_visitor.cpp",
        "src/ast/ast_generate.cpp",
        "src/ast/ast_simulate.cpp",
        "src/ast/ast_typedecl.cpp",
        "src/ast/ast_match.cpp",
        "src/ast/ast_module.cpp",
        "src/ast/ast_print.cpp",
        "src/ast/ast_aot_cpp.cpp",
        "src/ast/ast_infer_type.cpp",
        "src/ast/ast_lint.cpp",
        "src/ast/ast_allocate_stack.cpp",
        "src/ast/ast_derive_alias.cpp",
        "src/ast/ast_const_folding.cpp",
        "src/ast/ast_block_folding.cpp",
        "src/ast/ast_unused.cpp",
        "src/ast/ast_annotations.cpp",
        "src/ast/ast_export.cpp",
        "src/ast/ast_parse.cpp",
        "src/ast/ast_debug_info_helper.cpp",
        "src/ast/ast_handle.cpp"
    )

    -- module builtin
    add_files(
        "src/builtin/module_builtin.cpp",
        "src/builtin/module_builtin_misc_types.cpp",
        "src/builtin/module_builtin_runtime.cpp",
        "src/builtin/module_builtin_runtime_sort.cpp",
        "src/builtin/module_builtin_runtime_lockcheck.cpp",
        "src/builtin/module_builtin_vector.cpp",
        "src/builtin/module_builtin_vector_ctor.cpp",
        "src/builtin/module_builtin_array.cpp",
        "src/builtin/module_builtin_das.cpp",
        "src/builtin/module_builtin_math.cpp",
        "src/builtin/module_builtin_raster.cpp",
        "src/builtin/module_builtin_string.cpp",
        "src/builtin/module_builtin_rtti.cpp",
        "src/builtin/module_builtin_ast.cpp",
        "src/builtin/module_builtin_ast_serialize.cpp",
        "src/builtin/module_builtin_ast_flags.cpp",
        "src/builtin/module_builtin_ast_annotations.cpp",
        "src/builtin/module_builtin_ast_annotations_1.cpp",
        "src/builtin/module_builtin_ast_annotations_2.cpp",
        "src/builtin/module_builtin_ast_annotations_3.cpp",
        "src/builtin/module_builtin_ast_adapters.cpp",
        "src/builtin/module_builtin_uriparser.cpp",
        "src/builtin/module_jit.cpp",
        "src/builtin/module_builtin_fio.cpp",
        "src/builtin/module_builtin_dasbind.cpp",
        "src/builtin/module_builtin_network.cpp",
        "src/builtin/module_builtin_debugger.cpp",
        "src/builtin/module_builtin_jobque.cpp",
        "src/builtin/module_file_access.cpp"
    )

    -- misc
    add_files(
        "src/misc/sysos.cpp",
        "src/misc/string_writer.cpp",
        "src/misc/memory_model.cpp",
        "src/misc/job_que.cpp",
        "src/misc/free_list.cpp",
        "src/misc/daScriptC.cpp",
        "src/misc/uric.cpp"
    )

    -- fusion
    add_files(
        "src/simulate/simulate_fusion.cpp",
        "src/simulate/simulate_fusion_op1.cpp",
        "src/simulate/simulate_fusion_op1_return.cpp",
        "src/simulate/simulate_fusion_ptrfdr.cpp",
        "src/simulate/simulate_fusion_op2.cpp",
        "src/simulate/simulate_fusion_op2_set.cpp",
        "src/simulate/simulate_fusion_op2_bool.cpp",
        "src/simulate/simulate_fusion_op2_bin.cpp",
        "src/simulate/simulate_fusion_op2_vec.cpp",
        "src/simulate/simulate_fusion_op2_set_vec.cpp",
        "src/simulate/simulate_fusion_op2_bool_vec.cpp",
        "src/simulate/simulate_fusion_op2_bin_vec.cpp",
        "src/simulate/simulate_fusion_op2_scalar_vec.cpp",
        "src/simulate/simulate_fusion_at.cpp",
        "src/simulate/simulate_fusion_at_array.cpp",
        "src/simulate/simulate_fusion_tableindex.cpp",
        "src/simulate/simulate_fusion_misc_copy.cpp",
        "src/simulate/simulate_fusion_call1.cpp",
        "src/simulate/simulate_fusion_call2.cpp",
        "src/simulate/simulate_fusion_if.cpp"
    )

    -- simulate
    add_files(
        "src/hal/performance_time.cpp",
        "src/hal/debug_break.cpp",
        "src/hal/project_specific.cpp",
        "src/hal/project_specific_file_info.cpp",
        "src/misc/network.cpp",
        "src/simulate/hash.cpp",
        "src/simulate/debug_info.cpp",
        "src/simulate/runtime_string.cpp",
        "src/simulate/runtime_array.cpp",
        "src/simulate/runtime_table.cpp",
        "src/simulate/runtime_profile.cpp",
        "src/simulate/simulate.cpp",
        "src/simulate/simulate_exceptions.cpp",
        "src/simulate/simulate_gc.cpp",
        "src/simulate/simulate_tracking.cpp",
        "src/simulate/simulate_visit.cpp",
        "src/simulate/simulate_print.cpp",
        "src/simulate/simulate_fn_hash.cpp",
        "src/simulate/simulate_instrument.cpp",
        "src/simulate/heap.cpp",
        "src/simulate/data_walker.cpp",
        "src/simulate/debug_print.cpp",
        "src/simulate/json_print.cpp",
        "src/simulate/bin_serializer.cpp",
        "src/simulate/fs_file_info.cpp"
    )

target_end()