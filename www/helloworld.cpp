#include <iostream>

int main(){
    std::cout << "Status: 200\r\n"
        << "Content-type: text/html\r\n\r\n"
        << "<html>" << '\n'
        << "   <body>" << '\n'
        << "       Hello World!" << '\n'
        << '\n' << '\n'
        << "       <pre>Hello World!" << '\n'
        << '\n' << '\n'
        << "       Hello World!" << '\n'
        << '\n' << '\n'
        << "       Hello World!</pre>" << '\n'
        << '\n' << '\n'
        << "       Hello World!" << '\n'
        << "   </body>" << '\n'
        << "</html>" << '\n';

    return 0;
}
