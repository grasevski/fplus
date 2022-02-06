#include <cstdio>
#include <iostream>
#include <fstream>
#include <memory>
#include <llvm/ADT/StringSet.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include "gen/Absyn.H"
#include "gen/Parser.H"
#include "gen/Printer.H"

std::string path;
llvm::StringMap<std::unique_ptr<Modu>> modules;

static FnTy *compile(Modu&, const Ident&, const Fn&, llvm::StringSet<>&, bool);

static bool compile(const Ident& name) {
    const auto publish = modules.empty();
    const auto found = modules.find(name) != modules.end();
    auto& m = modules[name];

    if (found && !m) {
        std::cerr << "circular dependency: module " << name << " already included" << std::endl;
        return false;
    }

    auto f = fopen((path + name + ".fp").c_str(), "r");

    if (!f) {
        perror(name.c_str());
        return false;
    }

    std::unique_ptr<Modu> mod(dynamic_cast<Modu*>(pModule(f)));

    if (fclose(f)) {
        perror(name.c_str());
        return false;
    } else if (!mod) {
        std::cerr << "cannot parse module: " << name << std::endl;
        return false;
    }

    llvm::StringSet<> unused;

    for (const auto& x : *mod->listident_) {
        if (unused.contains(x)) {
            std::cerr << name << ": duplicate import: " << x << std::endl;
            return false;
        } else {
            unused.insert(x);
        }

        if (!compile(x)) {
            std::cerr << name << ": compile error: " << x << std::endl;
            return false;
        }
    }

    for (auto& x : *mod->listfn_3) {
        auto t = compile(*mod, name, *x, unused, publish);

        if (!t) {
            return false;
        }

        delete x;
        x = t;
    }

    m.swap(mod);

    if (!unused.empty()) {
        std::cerr << name << ": unused modules:";

        for (const auto& x : unused.keys())
            std::cerr << ' ' << x.str();

        std::cerr << std::endl;
        return false;
    }

    return true;
}

llvm::LLVMContext ctx;
llvm::IRBuilder<> builder(ctx);
llvm::Module mod("", ctx);

FnTy *compile(Modu& mod, const Ident& name, const Fn& fn, llvm::StringSet<>& unused, bool publish) {
    if (const auto func = dynamic_cast<const Func*>(&fn)) {
        return compile(mod, name, *func, unused, false);
    } else if (const auto fnpu = dynamic_cast<const FnPu*>(&fn)) {
        if (const auto func = dynamic_cast<const Func*>(fnpu->fn_)) {
            return compile(mod, name, *func, unused, publish);
        }
    }

    std::cerr << name << ": invalid function definition" << std::endl;
    return nullptr;
}

FnTy *compile(Modu& mod, const Ident& name, const Func& func, llvm::StringSet<>& unused, bool publish) {
    return new FnTy(func.clone(), nullptr);
}

Exp *compile(const Exp& exp) {
    if (const auto assign = dynamic_cast<const Eassign*>(&exp)) {
    } else if (const auto condition = dynamic_cast<const Econdition*>(&exp)) {
    } else if (const auto infix = dynamic_cast<const Einfix*>(&exp)) {
    } else if (const auto fakeinfix = dynamic_cast<const Efakeinfix*>(&exp)) {
    } else if (const auto preop = dynamic_cast<const Epreop*>(&exp)) {
    } else if (const auto array = dynamic_cast<const Earray*>(&exp)) {
    } else if (const auto funk = dynamic_cast<const Efunk*>(&exp)) {
    } else if (const auto casestatement = dynamic_cast<const Ecase*>(&exp)) {
    } else if (const auto list = dynamic_cast<const Elist*>(&exp)) {
    } else if (const auto map = dynamic_cast<const Emap*>(&exp)) {
    } else if (const auto var = dynamic_cast<const Evar*>(&exp)) {
    } else if (const auto constant = dynamic_cast<const Econst*>(&exp)) {
    } else if (const auto str = dynamic_cast<const Estring*>(&exp)) {
        return new Estring("str");
    }
}

Exp *unify(const Exp& a, const Exp& b) {
}

int main(int argc, const char* argv[]) {
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " FILE" << std::endl;
        return EXIT_FAILURE;
    }

    std::string name(argv[1]);
    const auto found = name.find_last_of("/\\");

    if (found != std::string::npos) {
        path = name.substr(0, found + 1);
        name = name.substr(found + 1);
    }

    if (!compile(name.substr(0, name.find_last_of(".")))) {
        std::cerr << "compile error" << std::endl;
        return EXIT_FAILURE;
    }

    llvm::verifyModule(mod);
    mod.print(llvm::outs(), nullptr);
    return EXIT_SUCCESS;
}
