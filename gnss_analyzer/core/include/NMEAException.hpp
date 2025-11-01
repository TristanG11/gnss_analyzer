#pragma once

#include <QString>

class NMEAException : public std::runtime_error {
public:
    explicit NMEAException(const QString &msg)
        : std::runtime_error(msg.toStdString()) {}
};

class ParsingError : public NMEAException {
public:
    explicit ParsingError(const QString &msg) : NMEAException("ParsingError: " + msg) {}
};

class InvalidDataError : public NMEAException {
public:
    explicit InvalidDataError(const QString &msg) : NMEAException("InvalidData: " + msg) {}
};
