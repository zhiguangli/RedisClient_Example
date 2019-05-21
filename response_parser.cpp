#include "response_parser.h"
#include <map>

AbstractReplyType::AbstractReplyType()
{

}

AbstractReplyType::~AbstractReplyType()
{

}

std::shared_ptr<AbstractReplyType> AbstractReplyType::createType(char c)
{
   std::map<char, std::function<AbstractReplyType*()>> factory_functions = {
        {'*', [](){ return new ArraysType; }},
        {'+', [](){ return new SimpleStringType; }},
        {'-', [](){ return new ErrorsType; }},
        {':', [](){ return new IntegersType; }},
        {'$', [](){ return new BulkStringType; }},
    };

    auto it = factory_functions.find(c);
    return ( it != factory_functions.end()) ? it->second()->shared_from_this() : nullptr;
}


ParseResult ArraysType::getChar(char c)
{
    switch(status_)
    {
        case ParseStatus::ParsingLength:
        {
            if(c != '\r')
            {
                if(std::isdigit(c) || (c == '-' && count_.size() == 0))
                {
                    count_.push_back(c);
                }
                else
                {
                    return ParseResult::Error;
                }
            }
            else
            {
                itemCount_ = std::stoi(count_);
                status_ = ParseStatus::ExpectLF;
            }
            return ParseResult::Continue;
            break;
        }
        case ParseStatus::ExpectLF:
        {
            if(c != '\n')
            {
                return ParseResult::Error;
            }
            status_ = ParseStatus::ParsingSubItemType;

            if(itemCount_ <= 0)
            {
                return ParseResult::Finished;
            }
            return ParseResult::Continue;
            break;
        }
        case ParseStatus::ParsingSubItemType:
        {
            currentItem_ = AbstractReplyType::createType(c);
            if(!currentItem_)
            {
                return ParseResult::Error;
            }

            items_.push_back(currentItem_);
            status_ = ParseStatus::ParsingSubItemContent;
            return ParseResult::Continue;
            break;
        }
        case ParseStatus::ParsingSubItemContent:
        {
            ParseResult pr = currentItem_->getChar(c);
            if(pr == ParseResult::Error)
            {
                return ParseResult::Error;
            }
            if(pr == ParseResult::Finished)
            {
                if(static_cast<int>(items_.size()) >= itemCount_)
                    return ParseResult::Finished;
                status_ = ParseStatus::ParsingSubItemType;
            }
            return  ParseResult::Continue;
            break;
        }
        default:
            return ParseResult::Error;
            break;
    }
}

ParseResult OneLineStringType::getChar(char c)
{
    switch(status_)
    {
        case ParseStatus::ParsingString:
        {
            if(c != '\r')
            {
                status_ = ParseStatus::ExpectLF;
            }
            else
            {
                content_.push_back(c);
            }
            return ParseResult::Continue;
            break;
        }
        case ParseStatus::ExpectLF:
        {
            if(c == '\n')
            {
                return ParseResult::Finished;
            }
            else
            {
                return ParseResult::Error;
            }
            break;
        }
        default:
            return ParseResult::Error;
            break;
    }
}

ParseResult IntegersType::getChar(char c)
{
    ParseResult pr = OneLineStringType::getChar(c);
    if(pr == ParseResult::Finished)
    {
        number_ = std::stoi(content_);
    }

    return pr;
}

ParseResult BulkStringType::getChar(char c)
{
    switch(status_)
    {
        case ParseStatus::ExpectLength:
        {
            if(c == '-' or std::isdigit(c))
            {
                lengthLine_.push_back(c);
            }
            else if(c == '\r')
            {
                status_ = ParseStatus::ExpectLengthLF;
            }
            else
            {
                // error
            }
            return ParseResult::Continue;
            break;
        }
        case ParseStatus::ExpectLengthLF:
        {
            if(c != '\n')
            {
                // error
            }
            length_ = std::stoi(lengthLine_);
            if(length_ == 0 || length_ == -1)
            {
                return ParseResult::Finished;
            }
            else if(length_ < 0)
            {
                // error
            }
            status_ = ParseStatus::ExpectContent;
            return ParseResult::Continue;
            break;
        }
        case ParseStatus::ExpectContent:
        {
            if(c == '\r')
            {
                status_ = ParseStatus::ExpectContentLF;
            }
            else
            {
                content_.push_back(c);
            }
            return ParseResult::Continue;
            break;
        }
        case ParseStatus::ExpectContentLF:
        {
            if(c != '\n')
            {
                //error
            }
            return ParseResult::Finished;
            break;
        }
        default:
            return ParseResult::Finished;
            break;
    }
}
