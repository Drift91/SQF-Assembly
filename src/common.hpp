#pragma once
#include <intercept.hpp>
#include <array>
#include <string_view>
#include <unordered_map>
#include <functional>
#include <optional>

using namespace std::string_view_literals;
using namespace intercept;


namespace intercept::__internal {

    class gsFuncBase {
    public:
        r_string _name;
        void copyPH(const gsFuncBase* other) noexcept {
            securityStuff = other->securityStuff;
            //std::copy(std::begin(other->securityStuff), std::end(other->securityStuff), std::begin(securityStuff));
        }
    private:
        std::array<size_t,
        #if _WIN64 || __X86_64__
            10
        #else
        #ifdef __linux__
            8
        #else
            11
        #endif
        #endif
        > securityStuff{};  //Will scale with x64
                            //size_t securityStuff[11];
    };
    class gsFunction : public gsFuncBase {
        void* placeholder12{ nullptr };//0x30  //jni function
    public:
        r_string _name2;//0x34 this is (tolower name)
        unary_operator * _operator;//0x38
    #ifndef __linux__
        r_string _rightType;//0x3c RString to something
        r_string _description;//0x38
        r_string _example;
        r_string _example2;
        r_string placeholder_11;
        r_string placeholder_12;
        r_string _category{ "intercept"sv }; //0x48
    #endif
                                             //const rv_string* placeholder13;
    };
    class gsOperator : public gsFuncBase {
        void* placeholder12{ nullptr };//0x30  JNI function
    public:
        r_string _name2;//0x34 this is (tolower name)
        int32_t placeholder_10{ 4 }; //0x38 Small int 0-5  priority
        binary_operator * _operator;//0x3c
    #ifndef __linux__
        r_string _leftType;//0x40 Description of left hand side parameter
        r_string _rightType;//0x44 Description of right hand side parameter
        r_string _description;//0x48
        r_string _example;//0x4c
        r_string placeholder_11;//0x60
        r_string _version;//0x64 some version number
        r_string placeholder_12;//0x68
        r_string _category{ "intercept"sv }; //0x6c
    #endif
    };
    class gsNular : public gsFuncBase {
    public:
        r_string _name2;//0x30 this is (tolower name)
        nular_operator * _operator;//0x34
    #ifndef __linux__
        r_string _description;//0x38
        r_string _example;
        r_string _example2;
        r_string _version;//0x44 some version number
        r_string placeholder_10;
        r_string _category; //0x4d
    #endif
        void* placeholder11{ nullptr };//0x50 JNI probably
        const char *get_map_key() const noexcept { return _name2.data(); }
    };



    class game_functions : public auto_array<gsFunction>, public gsFuncBase {
    public:
        game_functions(std::string name) : _name(name.c_str()) {}
        r_string _name;
        game_functions() noexcept {}
        const char *get_map_key() const noexcept { return _name.data(); }
    };

    class game_operators : public auto_array<gsOperator>, public gsFuncBase {
    public:
        game_operators(std::string name) : _name(name.c_str()) {}
        r_string _name;
        int32_t placeholder10{ 4 }; //0x2C Small int 0-5  priority
        game_operators() noexcept {}
        const char *get_map_key() const noexcept { return _name.data(); }
    };
}
using namespace intercept::__internal;

struct instructionVtable {
    void* vtbl[8];

    void init(game_instruction* engine, game_instruction* intercept) {
        auto in = *reinterpret_cast<instructionVtable**>(intercept);
        auto en = *reinterpret_cast<instructionVtable**>(engine);

        vtbl[0] = en->vtbl[-1];
        vtbl[1] = in->vtbl[0];
        vtbl[2] = in->vtbl[1];
        vtbl[3] = in->vtbl[2];
        vtbl[4] = en->vtbl[3];
        vtbl[5] = en->vtbl[4];
        vtbl[6] = en->vtbl[5];
        vtbl[7] = en->vtbl[6];
    }
};


class GameInstructionConst : public game_instruction {
public:
    game_value value;
    static const size_t typeIDHash = 0x8c0dbf90;
    static inline instructionVtable defVtable;
    void setVtable() {
        auto vtbl = reinterpret_cast<instructionVtable*>(&defVtable.vtbl[1]);
        *reinterpret_cast<instructionVtable**>(this) = vtbl;
    }


    GameInstructionConst(game_value&& val) : value(val) { setVtable(); }
    GameInstructionConst() { setVtable(); }
    GameInstructionConst(std::nullptr_t) {}
    static ref<GameInstructionConst> make(game_value val) {
        return new GameInstructionConst(std::move(val));
        //return rv_allocator<GameInstructionConst>::create_single(std::move(val));
    }
    template<class... _Types>
    static ref<GameInstructionConst> make(_Types&&... _Args) {
        return new GameInstructionConst(std::forward<_Types>(_Args)...);
        //return rv_allocator<GameInstructionConst>::create_single(std::forward<_Types>(_Args)...);
    }
    //virtual void lastRefDeleted() const { rv_allocator<GameInstructionConst>::destroy_deallocate(const_cast<GameInstructionConst*>(this)); }
    virtual bool exec(game_state& state, vm_context& t) { return false; }
    virtual int stack_size(void* t) const { return 0; }
    virtual r_string get_name() const { return ""sv; }

};

class GameInstructionVariable : public game_instruction {
public:
    r_string name;

    static const size_t typeIDHash = 0xc04f83b1;
    static inline instructionVtable defVtable;
    void setVtable() {
        auto vtbl = reinterpret_cast<instructionVtable*>(&defVtable.vtbl[1]);
        *reinterpret_cast<instructionVtable**>(this) = vtbl;
    }

    GameInstructionVariable() { setVtable(); }
    GameInstructionVariable(r_string gf) : name(gf) { setVtable(); }
    GameInstructionVariable(std::nullptr_t) {}

    template<class... _Types>
    static ref<GameInstructionVariable> make(_Types&&... _Args) {
        return rv_allocator<GameInstructionVariable>::create_single(std::forward<_Types>(_Args)...);
    }
    virtual void lastRefDeleted() const { rv_allocator<GameInstructionVariable>::destroy_deallocate(const_cast<GameInstructionVariable*>(this)); }
    virtual bool exec(game_state& state, vm_context& t) { return false; }
    virtual int stack_size(void* t) const { return 0; }
    virtual r_string get_name() const { return ""sv; }

};

class GameInstructionOperator : public game_instruction {
public:
    const game_operators *_operators;

    static const size_t typeIDHash = 0x0ac32571;
    static inline instructionVtable defVtable;
    void setVtable() {
        auto vtbl = reinterpret_cast<instructionVtable*>(&defVtable.vtbl[1]);
        *reinterpret_cast<instructionVtable**>(this) = vtbl;
    }

    template<class... _Types>
    static ref<GameInstructionOperator> make(_Types&&... _Args) {
        return rv_allocator<GameInstructionOperator>::create_single(std::forward<_Types>(_Args)...);
    }
    virtual void lastRefDeleted() const { rv_allocator<GameInstructionOperator>::destroy_deallocate(const_cast<GameInstructionOperator*>(this)); }
    virtual bool exec(game_state& state, vm_context& t) { return false; }
    virtual int stack_size(void* t) const { return 0; }
    virtual r_string get_name() const { return ""sv; }

    GameInstructionOperator() { setVtable(); }
    GameInstructionOperator(const game_operators* gf) : _operators(gf) { setVtable(); }
    GameInstructionOperator(std::nullptr_t) {}

    r_string getFuncName() const {
        return _operators->_name;
    }


};

class GameInstructionFunction : public game_instruction {
public:
    const game_functions *_functions;

    static const size_t typeIDHash = 0x72ff7d2d;
    static inline instructionVtable defVtable;
    void setVtable() {
        auto vtbl = reinterpret_cast<instructionVtable*>(&defVtable.vtbl[1]);
        *reinterpret_cast<instructionVtable**>(this) = vtbl;
    }

    template<class... _Types>
    static ref<GameInstructionFunction> make(_Types&&... _Args) {
        return rv_allocator<GameInstructionFunction>::create_single(std::forward<_Types>(_Args)...);
    }
    virtual void lastRefDeleted() const { rv_allocator<GameInstructionFunction>::destroy_deallocate(const_cast<GameInstructionFunction*>(this)); }
    virtual bool exec(game_state& state, vm_context& t) { return false; }
    virtual int stack_size(void* t) const { return 0; }
    virtual r_string get_name() const { return ""sv; }

    GameInstructionFunction() { setVtable(); }
    GameInstructionFunction(const game_functions* gf) : _functions(gf) { setVtable(); }
    GameInstructionFunction(std::nullptr_t) {}

    r_string getFuncName() const {
        return _functions->_name;
    }



};

class GameInstructionArray : public game_instruction {
public:
    int size;

    static const size_t typeIDHash = 0x4b5efb7a;
    static inline instructionVtable defVtable;
    void setVtable() {
        auto vtbl = reinterpret_cast<instructionVtable*>(&defVtable.vtbl[1]);
        *reinterpret_cast<instructionVtable**>(this) = vtbl;
    }

    template<class... _Types>
    static ref<GameInstructionArray> make(_Types&&... _Args) {
        return rv_allocator<GameInstructionArray>::create_single(std::forward<_Types>(_Args)...);
    }
    virtual void lastRefDeleted() const { rv_allocator<GameInstructionArray>::destroy_deallocate(const_cast<GameInstructionArray*>(this)); }
    virtual bool exec(game_state& state, vm_context& t) { return false; }
    virtual int stack_size(void* t) const { return 0; }
    virtual r_string get_name() const { return ""sv; }

    GameInstructionArray() { setVtable(); }
    GameInstructionArray(std::nullptr_t) {}
    GameInstructionArray(int sz) : size(sz) { setVtable(); }
};

class GameInstructionAssignment : public game_instruction {
public:
    r_string name;
    bool forceLocal;

    static const size_t typeIDHash = 0xd27a68ec;
    static inline instructionVtable defVtable;
    void setVtable() {
        auto vtbl = reinterpret_cast<instructionVtable*>(&defVtable.vtbl[1]);
        *reinterpret_cast<instructionVtable**>(this) = vtbl;
    }

    template<class... _Types>
    static ref<GameInstructionAssignment> make(_Types&&... _Args) {
        return rv_allocator<GameInstructionAssignment>::create_single(std::forward<_Types>(_Args)...);
    }
    virtual void lastRefDeleted() const { rv_allocator<GameInstructionAssignment>::destroy_deallocate(const_cast<GameInstructionAssignment*>(this)); }
    virtual bool exec(game_state& state, vm_context& t) { return false; }
    virtual int stack_size(void* t) const { return 0; }
    virtual r_string get_name() const { return ""sv; }

    GameInstructionAssignment() { setVtable(); }
    GameInstructionAssignment(std::nullptr_t) {}
    GameInstructionAssignment(r_string nm, bool local) : name(nm), forceLocal(local) { setVtable(); }
};

class GameInstructionNewExpression : public game_instruction {
public:
    int beg{ 0 };
    int end{ 0 };


    static const size_t typeIDHash = 0xc2bb0eeb;
    static inline instructionVtable defVtable;
    void setVtable() {
        auto vtbl = reinterpret_cast<instructionVtable*>(&defVtable.vtbl[1]);
        *reinterpret_cast<instructionVtable**>(this) = vtbl;
    }

    template<class... _Types>
    static ref<GameInstructionNewExpression> make(_Types&&... _Args) {
        return new GameInstructionNewExpression(std::forward<_Types>(_Args)...);
        //return rv_allocator<GameInstructionNewExpression>::create_single(std::forward<_Types>(_Args)...);
    }
    //virtual void lastRefDeleted() const { rv_allocator<GameInstructionNewExpression>::destroy_deallocate(const_cast<GameInstructionNewExpression*>(this)); }
    virtual bool exec(game_state& state, vm_context& t) { return false; }
    virtual int stack_size(void* t) const { return 0; }
    virtual r_string get_name() const { return ""sv; }

    GameInstructionNewExpression() { setVtable(); }
    GameInstructionNewExpression(std::nullptr_t) {}
};

namespace intercept::assembly {

	class asshelper
	{
	private:
		std::unordered_map<std::string_view, game_value> nmap;
		std::unordered_map<std::string_view, std::function<std::optional<game_value>(game_value)>> umap;
		std::unordered_map<std::string_view, std::function<std::optional<game_value>(game_value, game_value)>> bmap;
	public:
		asshelper(game_state* gs);
		bool containsNular(std::string_view key) const;
		bool containsUnary(std::string_view key) const;
		bool containsBinary(std::string_view key) const;
		game_value get(std::string_view key) const;
		std::optional<game_value> asshelper::get(game_state* gs, std::string_view key, ref<game_instruction> right) const;
		std::optional<game_value> asshelper::get(game_state* gs, std::string_view key, ref<game_instruction> left, ref<game_instruction> right) const;
	};
    void optimize(game_state* gs, asshelper* nh, ref<compact_array<ref<game_instruction>>> instructions);
}