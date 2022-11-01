/*
 * Copyright (c) 2022 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#include <string>
#include <functional>
#include <hash_map.h>
#include <string_conversions.h>

namespace spl {

/**
 * @brief Describes a command-line argument for use with ArgumentParser.
 */
class Argument {

friend class ArgumentParser;

private:

    std::string _argument;
    int _numParams;
    std::function<bool(const char * const *)> _action;

public:

    /**
     * @brief Construct a new Argument object
     * 
     */
    Argument()
    :   _numParams(0),
        _action(nullptr)
    {}

    /**
     * @brief Construct a new Argument object
     * 
     * @param argument Expected command-line argument.
     */
    Argument(const std::string &argument)
    :   _argument(argument),
        _numParams(0),
        _action(nullptr)
    { }

    /**
     * @brief Construct a new Argument object
     * 
     * @param argument Expected command-line argument.
     */
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

    /**
     * @brief Sets the number of parameters that follow this argument.
     * 
     * @param val Number of parameters.
     * @return A reference to this object for chaining.
     */
    Argument & numParams(int val) {
        _numParams = val;
        return *this;
    }

    /**
     * @brief Sets the action to be executed when this command-line argument is
     * encountered.
     * 
     * @param val Callback function for argument processing. This function
     * should return a boolean to indicate to indicate success/failure.
     * @return A reference to this object for chaining.
     */
    Argument & action(const std::function<bool(const char * const *)> &val) {
        _action = val;
        return *this;
    }

    /**
     * @brief An argument action that stores the argument parameter into a
     * variable. This works for boolean, any numeric type, and any type that can
     * be copy-assigned and const char *.
     * 
     * @param dest A reference to the variable to store the parsed parameter
     * value.
     * @return An argument action function.
     */
    template <
        typename T,
        std::enable_if_t<
            ! (std::is_same_v<T, uint8>
            || std::is_same_v<T, uint16>
            || std::is_same_v<T, uint32>
            || std::is_same_v<T, uint64>
            || std::is_same_v<T, int8>
            || std::is_same_v<T, int16>
            || std::is_same_v<T, int32>
            || std::is_same_v<T, int64>
            || std::is_same_v<T, float32>
            || std::is_same_v<T, float64>
            || std::is_same_v<T, float128>
            ),
            int
        > = 0
    >
    static std::function<bool(const char * const *)> store(T &dest) {
        return std::function([&dest] (const char * const *args)->bool {
            try {
                dest = args[0];
            }
            catch (...) {
                return false;
            }
            return true;
        });
    }

    /**
     * @brief An argument action that stores the argument parameter into a
     * variable. This works for boolean, any numeric type, and any type that can
     * be copy-assigned and const char *.
     * 
     * @param dest A reference to the variable to store the parsed parameter
     * value.
     * @return An argument action function.
     */
    template <
        typename T,
        std::enable_if_t<
            std::is_same_v<T, uint8>
            || std::is_same_v<T, uint16>
            || std::is_same_v<T, uint32>
            || std::is_same_v<T, uint64>
            || std::is_same_v<T, int8>
            || std::is_same_v<T, int16>
            || std::is_same_v<T, int32>
            || std::is_same_v<T, int64>
            || std::is_same_v<T, float32>
            || std::is_same_v<T, float64>
            || std::is_same_v<T, float128>,
            int
        > = 0
    >
    static std::function<bool(const char * const *)> store(T &dest) {
        return std::function([&dest] (const char * const *args)->bool {
            try {
                dest = StringConversions::parse<T>(args[0]);
            }
            catch (...) {
                return false;
            }
            return true;
        });
    }

    /**
     * @brief An argument action that sets a variable with the specified value.
     * 
     * @param dest A reference to the variable to assign the specified value to.
     * @param val The value to assign.
     * @return An argument action function.
     */
    template <typename T, typename U>
    static std::function<bool(const char * const *)> set(T &dest, U val) {
        return std::function([&dest, val] (const char * const *args)->bool {
            dest = val;
            return true;
        });
    }
};

template <>
std::function<bool(const char * const *)> Argument::store<bool, 0>(bool &val) {
    return std::function([&val] (const char * const *args)->bool {
        if (strcasecmp(args[0], "true") == 0) val = true;
        else if (strcasecmp(args[0], "false") == 0) val = false;
        else return false;

        return true;
    });
}

/**
 * @brief A command-line argument parser.
 */
class ArgumentParser {

private:

    HashMap<std::string, Argument> _args;

public:

    ArgumentParser() = default;

    /**
     * @brief Construct a new ArgumentParser object.
     * 
     * @param arguments A list of Argument objects.
     */
    ArgumentParser(const std::initializer_list<Argument> &arguments);

    ArgumentParser(const ArgumentParser &) = delete;

    ArgumentParser(ArgumentParser &&) = delete;

    ~ArgumentParser() = default;

    ArgumentParser & operator=(const ArgumentParser &) = delete;

    ArgumentParser & operator=(ArgumentParser &&) = delete;

    /**
     * @brief Adds an Argument object to the list of registered arguments.
     * 
     * @param arg An Argument object.
     * @return A reference to this object for chaining.
     */
    ArgumentParser & add(const Argument &arg) {
        _args.put(arg._argument, arg);
        return *this;
    }

    /**
     * @brief Adds an Argument object to the list of registered arguments.
     * 
     * @param arg An Argument object.
     * @return A reference to this object for chaining.
     */
    ArgumentParser & add(Argument &&arg) {
        _args.put(arg._argument, std::move(arg));
        return *this;
    }

    /**
     * @brief Parses the command-line arguments specified by argc and argv. The
     * command-line arguments are matched with the set of registered arguments.
     * Note: argv[0] is assumed to be the executable path and is ignored by this
     * function.
     * 
     * @param argc The argument count parameter passed to main().
     * @param argv The argument vector parameter passed to main().
     * @return A reference to this object for chaining.
     */
    ArgumentParser & parse(int argc, const char * const * argv);
};

}
