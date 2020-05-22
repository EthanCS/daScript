#include "daScript/misc/platform.h"

#include "daScript/ast/ast.h"

void das_yybegin(const char * str);
int das_yyparse();
int das_yylex_destroy();

namespace das {

    bool isUtf8Text ( const char * src, uint32_t length ) {
        if ( length>=3  ) {
            auto usrc = (const uint8_t *)src;
            if ( usrc[0]==0xef && usrc[1]==0xbb && usrc[2]==0xbf) {
                return true;
            }
        }
        return false;
    }

    __forceinline bool isalphaE ( int ch ) {
        return (ch>='a' && ch<='z') || (ch>='A' && ch<='Z');
    }

    __forceinline bool isalnumE ( int ch ) {
        return (ch>='0' && ch<='9') || (ch>='a' && ch<='z') || (ch>='A' && ch<='Z');
    }

    vector<string> getAllRequie ( const char * src, uint32_t length ) {
        if ( isUtf8Text(src,length) ) { // skip utf8 byte order mark
            src += 3;
            length -= 3;
        }
        vector<string> req;
        const char * src_end = src + length;
        bool wb = true;
        while ( src < src_end ) {
            if ( src[0]=='"' ) {
                src ++;
                while ( src < src_end && src[0]!='"' ) {
                    src ++;
                }
                src ++;
                wb = true;
                continue;
            } else if ( src[0]=='/' && src[1]=='/' ) {
                while ( src < src_end && !(src[0]=='\n') ) {
                    src ++;
                }
                src ++;
                wb = true;
                continue;
            } else if ( src[0]=='/' && src[1]=='*' ) {
                int depth = 0;
                while ( src < src_end ) {
                    if ( src[0]=='/' && src[1]=='*' ) {
                        depth ++;
                        src += 2;
                    } else if ( src[0]=='*' && src[1]=='/' ) {
                        if ( --depth==0 ) break;
                        src += 2;
                    } else {
                        src ++;
                    }
                }
                src +=2;
                wb = true;
                continue;
            } else if ( wb && ((src+8)<src_end) && src[0]=='r') {   // need space for 'require '
                if ( memcmp(src, "require", 7)==0 ) {
                    src += 7;
                    if ( isspace(src[0]) ) {
                        while ( src < src_end && isspace(src[0]) ) {
                            src ++;
                        }
                        if ( src >= src_end ) {
                            continue;
                        }
                        if ( src[0]=='_' || isalphaE(src[0]) || src[0] ) {
                            string mod;
                            while ( src < src_end && (isalnumE(src[0]) || src[0]=='_' || src[0]=='.' || src[0]=='/') ) {
                                mod += *src ++;
                            }
                            req.push_back(mod);
                            continue;
                        } else {
                            wb = true;
                            goto nextChar;
                        }
                    } else {
                        wb = false;
                        goto nextChar;
                    }
                } else {
                    goto nextChar;
                }
            }
        nextChar:
            wb = src[0]!='_' && (wb ? !isalnumE(src[0]) : !isalphaE(src[0]));
            src ++;
        }
        return req;
    }

    string getModuleName ( const string & nameWithDots ) {
        auto idx = nameWithDots.find_last_of("./");
        if ( idx==string::npos ) return nameWithDots;
        return nameWithDots.substr(idx+1);
    }

    string getModuleFileName ( const string & nameWithDots ) {
        auto fname = nameWithDots;
        // TODO: should we?
        replace ( fname.begin(), fname.end(), '.', '/' );
        return fname;
    }

    bool getPrerequisits ( const string & fileName,
                          const FileAccessPtr & access,
                          vector<ModuleInfo> & req,
                          vector<string> & missing,
                          vector<string> & circular,
                          das_set<string> & dependencies,
                          ModuleGroup & libGroup,
                          TextWriter & log,
                          int tab ) {
        if ( auto fi = access->getFileInfo(fileName) ) {
            log << string(tab,'\t') << "in " << fileName << "\n";
            vector<string> ownReq = getAllRequie(fi->source, fi->sourceLength);
            for ( auto & mod : ownReq ) {
                log << string(tab,'\t') << "require " << mod << "\n";
                auto module = Module::require(mod); // try native with that name
                if ( !module ) {
                    auto info = access->getModuleInfo(mod, fileName);
                    if ( !info.moduleName.empty() ) {
                        mod = info.moduleName;
                        log << string(tab,'\t') << " resolved as " << mod << "\n";
                    }
                    auto it_r = find_if(req.begin(), req.end(), [&] ( const ModuleInfo & reqM ) {
                        return reqM.moduleName == mod;
                    });
                    if ( it_r==req.end() ) {
                        if ( dependencies.find(mod) != dependencies.end() ) {
                            // circular dependency
                            log << string(tab,'\t') << "from " << fileName << " require " << mod << " - CIRCULAR DEPENDENCY\n";
                            circular.push_back(mod);
                            return false;
                        }
                        dependencies.insert(mod);
                        // module file name
                        if ( info.moduleName.empty() ) {
                            // request can't be translated to module name
                            log << string(tab,'\t') << "from " << fileName << " require " << mod << " - MODULE INFO NOT FOUND\n";
                            missing.push_back(mod);
                            return false;
                        }
                        if ( !getPrerequisits(info.fileName, access, req, missing, circular, dependencies, libGroup, log, tab + 1) ) {
                            return false;
                        }
                        log << string(tab,'\t') << "from " << fileName << " require " << mod
                            << " - ok, new module " << info.moduleName << " at " << info.fileName << "\n";
                        req.push_back(info);
                    } else {
                        log << string(tab,'\t') << "from " << fileName << " require " << mod << " - already required\n";
                    }
                } else {
                    log << string(tab,'\t') << "from " << fileName << " require " << mod << " - ok\n";
                    libGroup.addModule(module);
                }
            }
            return true;
        } else {
            log << string(tab,'\t') << "in " << fileName << " - NOT FOUND\n";
            missing.push_back(fileName);
            return false;
        }
    }

    // PARSER

    ProgramPtr g_Program;
    FileAccessPtr g_Access;
    vector<FileInfo *>  g_FileAccessStack;

    extern "C" int64_t ref_time_ticks ();
    extern "C" int get_time_usec (int64_t reft);

    ProgramPtr parseDaScript ( const string & fileName,
                              const FileAccessPtr & access,
                              TextWriter & logs,
                              ModuleGroup & libGroup,
                              bool exportAll,
                              CodeOfPolicies policies ) {
        auto time0 = ref_time_ticks();
        int err;
        auto program = g_Program = make_smart<Program>();
        program->isCompiling = true;
        g_Program->policies = policies;
        g_Access = access;
        program->thisModuleGroup = &libGroup;
        libGroup.foreach([&](Module * pm){
            g_Program->library.addModule(pm);
            return true;
        },"*");
        g_FileAccessStack.clear();
        if ( auto fi = access->getFileInfo(fileName) ) {
            g_FileAccessStack.push_back(fi);
            if (isUtf8Text(fi->source, fi->sourceLength)) {
                das_yybegin(fi->source + 3);
            } else {
                das_yybegin(fi->source);
            }
        } else {
            g_Program->error(fileName + " not found", "","",LineInfo());
            g_Program.reset();
            g_Access.reset();
            g_FileAccessStack.clear();
            program->isCompiling = false;
            return program;
        }
        err = das_yyparse();        // TODO: add mutex or make thread safe?
        das_yylex_destroy();
        g_Program.reset();
        g_Access.reset();
        g_FileAccessStack.clear();
        if ( err || program->failed() ) {
            sort(program->errors.begin(),program->errors.end());
            program->isCompiling = false;
            return program;
        } else {
            program->inferTypes(logs, libGroup);
            if ( !program->failed() ) {
                program->lint(libGroup);
                if (program->getOptimize()) {
                    program->optimize(logs,libGroup);
                } else {
                    program->buildAccessFlags(logs);
                }
                if (!program->failed())
                    program->verifyAndFoldContracts();
                if (!program->failed())
                    program->markOrRemoveUnusedSymbols(exportAll);
                if (!program->failed())
                    program->allocateStack(logs);
                if (!program->failed())
                    program->finalizeAnnotations();
            }
            if (!program->failed()) {
                if (program->options.getBoolOption("log")) {
                    logs << *program;
                }
                if (program->options.getBoolOption("plot")) {
                    logs << "\n" << program->dotGraph() << "\n";
                }
            }
            sort(program->errors.begin(), program->errors.end());
            if ( program->options.getBoolOption("log_compile_time",false) ) {
                auto dt = get_time_usec(time0) / 1000000.;
                logs << "compiler took " << dt << "\n";
            }
            program->isCompiling = false;
            return program;
        }
    }

    ProgramPtr compileDaScript ( const string & fileName,
                                const FileAccessPtr & access,
                                TextWriter & logs,
                                ModuleGroup & libGroup,
                                bool exportAll,
                                CodeOfPolicies policies ) {
        vector<ModuleInfo> req;
        vector<string> missing, circular;
        das_set<string> dependencies;
        TextWriter tw;
        if ( getPrerequisits(fileName, access, req, missing, circular, dependencies, libGroup, tw, 1) ) {
            for ( auto & mod : req ) {
                if ( !libGroup.findModule(mod.moduleName) ) {
                    auto program = parseDaScript(mod.fileName, access, logs, libGroup, true, policies);
                    if ( program->failed() ) {
                        return program;
                    }
                    if ( program->thisModule->name.empty() ) {
                        program->thisModule->name = mod.moduleName;
                    }
                    libGroup.addModule(program->thisModule.release());
                    program->library.foreach([&](Module * pm) -> bool {
                        if ( !pm->name.empty() && pm->name!="$" ) {
                            if ( !libGroup.findModule(pm->name) ) {
                                libGroup.addModule(pm);
                            }
                        }
                        return true;
                    }, "*");
                }
            }
            auto res = parseDaScript(fileName, access, logs, libGroup, exportAll, policies);
            if ( res->options.getBoolOption("log_require",false) ) {
                logs << "module dependency graph:\n" << tw.str();
            }
            return res;
        } else {
            auto program = make_smart<Program>();
            program->policies = policies;
            program->thisModuleGroup = &libGroup;
            TextWriter err;
            for ( auto & mis : missing ) {
                err << "missing prerequisit " << mis << "\n";
            }
            for ( auto & mis : circular ) {
                err << "circular dependency " << mis << "\n";
            }
            program->error(err.str(),"module dependency graph:\n" + tw.str(), "", LineInfo(),
                            CompilationError::module_not_found);
            return program;
        }
    }
}

