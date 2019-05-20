#include <iostream>
#include "response_parser.h"

using namespace std;



int main()
{

    std::string content = "*5\r\n:1\r\n:2\r\n:3\r\n:4\r\n$6\r\nfoobar\r\n";

    auto replyType = AbstractReplyType::createType(content.at(0));

    if(!replyType)
    {
        for(auto i = 1; i < content.size(); ++i)
        {
            switch(replyType->getChar(content.at(i)))
            {
                case ParseResult::Error:
                    // reply error string to consloe.
                    std::cout << "parse error" << std::endl;
                    break;
                case ParseResult::Finished:
                    // reply to console.
                    std::cout << replyType->toString();
                    break;
            }
        }
    }

    return 0;
}
