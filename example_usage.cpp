#include <iostream>
#include "response_parser.h"

using namespace std;



int main()
{

    std::string content = "*5\r\n:1\r\n:2\r\n:3\r\n:4\r\n$6\r\nfoobar\r\n";

    auto replyItem = AbstractReplyType::createType(content.at(0));

    if(!replyItem)
    {
        for(auto i = 1; i < content.size(); ++i)
        {
            switch(replyItem->getChar(content.at(i)))
            {
                case ParseResult::Error:
                    // reply error string to consloe.
                    break;
                case ParseResult::Finished:
                    // reply to console.
                    break;
            }
        }
    }

    return 0;
}
