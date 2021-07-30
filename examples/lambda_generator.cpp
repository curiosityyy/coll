#include <iostream>
#include <iomanip>
#include <sstream>

#include "coll/coll.hpp"

int main() {
  std::cout << "#pragma once" << std::endl
    << "/**" << std::endl
    << " * Generated by examples/lambda_generator.cpp" << std::endl
    << " * anony stands for anonymous" << std::endl
    << " * r stands for reference" << std::endl
    << " * c stands for copy" << std::endl
    << " * a stands for any" << std::endl
    << " * v stands for void" << std::endl
    << " * m stands for mutable" << std::endl
    << " **/" << std::endl;

  coll::elements(std::make_pair("", ""), std::make_pair("c", ""), std::make_pair("r", "&"), std::make_pair("a", "&&"))
    | coll::foreach([&](auto input) {
        coll::elements(std::make_pair("", ' '), std::make_pair("r", '&'), std::make_pair("c", '='))
          | coll::foreach([&](auto capture) {
              coll::elements(std::make_pair("v", ""), std::make_pair("c", ""), std::make_pair("r", "auto&"), std::make_pair("a", "decltype(auto)"))
                | coll::foreach([&](auto output) {
                    std::ostringstream name;
                    name << "anony" << capture.first << '_' << input.first << output.first << "(...)";

                    std::ostringstream in;
                    in << std::setw(4) << (input.first[0] ? "auto" : "")
                       << std::left << std::setw(3) << input.second
                       << (input.first[0] ? "_" : " ");

                    std::ostringstream out;
                    out << std::setw(4) << (output.second[0] ? " -> " : "")
                        << std::left << std::setw(14) << output.second;

                    std::ostringstream body;
                    body << (output.first[0] == 'v' ? "__VA_ARGS__" : "return (__VA_ARGS__);");

                    { // without mutable
                      std::cout << "#define "
                        << std::left << std::setw(15) << name.str()
                        << " ([" << capture.second << "](" << in.str() << ")"
                        << std::setw(4+14+9) << out.str() << "{ "
                        << body.str() << " })"
                        << std::endl;
                    }

                    if (capture.first[0] == 'c') {
                      // with mutable
                      name.seekp(name.tellp() - std::streamoff(5));
                      name << "m(...)";
                      out << " mutable";
                      std::cout << "#define "
                        << std::left << std::setw(15) << name.str()
                        << " ([" << capture.second << "](" << in.str() << ")"
                        << std::setw(4+14+9) << out.str() << "{ "
                        << body.str() << " })"
                        << std::endl;

                    }
                  });
            });
      });
}
