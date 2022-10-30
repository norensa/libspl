#pragma once

#include <string>
#include <functional>
#include <hash_map.h>
#include <string_conversions.h>

namespace spl {

class Argument {

friend class ArgumentParser;

private:

    std::string _argument;
    int _numParams;
    std::function<bool(const char * const *)> _action;

public:

    Argument()
    :   _numParams(0),
        _action(nullptr)
    {}

    Argument(const std::string &argument)
    :   _argument(argument),
        _numParams(0),
        _action(nullptr)
    { }

    Argument(std::string &&argument)
    :   _argument(std::move(argument)),
        _numParams(0),
        _action(nullptr)
    { }

    Argument(const Argument &) = default;

    Argument(Argument &&) = default;

    ~Argument() = default;

    Argument & operator=(const Argument &) = default;

    Argument & operator=(Argument &&) = default;

    Argument & numParams(int val) {
        _numParams = val;
        return *this;
    }

    Argument & action(const std::function<bool(const char * const *)> &val) {
        _action = val;
        return *this;
    }

    template <typename T>
    static std::function<bool(const char * const *)> store(T &val) {
        return std::function([&val] (const char * const *args)->bool {
            try {
                val = StringConversions::parse<T>(args[0]);
            }
            catch (...) {
                return false;
            }
            return true;
        });
    }

    template <typename T, typename U>
    static std::function<bool(const char * const *)> set(T &destination, U val) {
        return std::function([&destination, val] (const char * const *args)->bool {
            destination = val;
            return true;
        });
    }
};

template <>
std::function<bool(const char * const *)> Argument::store<bool>(bool &val) {
    return std::function([&val] (const char * const *args)->bool {
        if (strcasecmp(args[0], "true") == 0) val = true;
        else if (strcasecmp(args[0], "false") == 0) val = false;
        else return false;

        return true;
    });
}

template <>
std::function<bool(const char * const *)> Argument::store<std::string>(std::string &val) {
    return std::function([&val] (const char * const *args)->bool {
        val = args[0];
        return true;
    });
}


class ArgumentParser {

private:

    HashMap<std::string, Argument> _args;

public:

    ArgumentParser() = default;

    ArgumentParser(const std::initializer_list<Argument> &arguments);

    ArgumentParser(const ArgumentParser &) = delete;

    ArgumentParser(ArgumentParser &&) = delete;

    ~ArgumentParser() = default;

    ArgumentParser & operator=(const ArgumentParser &) = delete;

    ArgumentParser & operator=(ArgumentParser &&) = delete;

    ArgumentParser & add(const Argument &arg) {
        _args.put(arg._argument, arg);
        return *this;
    }

    ArgumentParser & add(Argument &&arg) {
        _args.put(arg._argument, std::move(arg));
        return *this;
    }

    ArgumentParser & parse(int argc, const char * const * argv);
};

}
