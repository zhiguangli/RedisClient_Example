#ifndef RESPONSE_PARSER_H
#define RESPONSE_PARSER_H

#include <memory>
#include <functional>
#include <vector>

class ArraysType;
class OneLineStringType;
class BulkStringType;
class SimpleStringType;
class ErrorsType;
class IntegersType;

enum class ParseResult
{
    Init,
    Continue,
    Finished,
    Error
};


class AbstractReplyType : public std::enable_shared_from_this<AbstractReplyType>
{
public:
    AbstractReplyType();
    virtual ~AbstractReplyType();

    virtual std::string toString() = 0;
    virtual ParseResult feed(char c) = 0;

    //factory method
    static std::shared_ptr<AbstractReplyType> createType(char c);
};


class ArraysType : public AbstractReplyType
{
    enum class ParseStatus
    {
        ParsingLength,          //parsing length,
        ExpectLF,               //got \r, expect \n
        ParsingSubItemType,     //expect $ + - :
        ParsingSubItemContent   //parsing sub item content.
    };
public:

    ArraysType() : AbstractReplyType(), status_(ParseStatus::ParsingLength) {}

    std::string toString() override
    {
        std::string result("[");
        for(std::size_t i = 0; i < items_.size(); ++i)
        {
            if(i > 0)
                result.append(", ");

            result.append(items_[i]->toString());
        }

        result.append("]");
        return result;
    }

    ParseResult feed(char c) override;

private:
    int itemCount_;
    std::string count_;
    ParseStatus status_;
    std::shared_ptr<AbstractReplyType> currentItem_;
    std::vector<std::shared_ptr<AbstractReplyType>> items_;
};

class OneLineStringType : public AbstractReplyType
{
    enum class ParseStatus
    {
        ParsingString,
        ExpectLF,
    };
public:


    std::string toString() override
    {
        return content_;
    }

    ParseResult feed(char c) override;

protected:
    ParseStatus status_ = ParseStatus::ParsingString;
    std::string content_;
};

class BulkStringType : public AbstractReplyType
{
    enum class ParseStatus
    {
        ExpectLength,       // first line, 0 for empty string, -1 for null
        ExpectLengthLF,     // first line of \n
        ExpectContent,
        ExpectContentLF,    // next line of \n
    };
public:
    std::string toString() override
    {
        if(length_ == -1)
            return "(nil)";

        return std::string("\"") + content_ + std::string("\"");
    }
    ParseResult feed(char c) override;

private:
    ParseStatus status_ = ParseStatus::ExpectLength;
    int length_ = 0;
    std::string lengthLine_;
    std::string content_;
};

class ErrorsType : public OneLineStringType
{
public:
    std::string toString() override
    {
        return std::string("(error) ") + content_;
    }
};

class IntegersType : public OneLineStringType
{
public:
    std::string toString() override
    {
        return std::string("(integer) ") + std::to_string(number_);
    }
    ParseResult feed(char c) override;

private:
    int number_ = -1;
};

class SimpleStringType : public OneLineStringType
{
public:
};

#endif // RESPONSE_PARSER_H
