#pragma once

#include <string>
#include <initializer_list>
#include <string_view>
#include <vector>

#include "head/generator.hpp"
#include "head/elseif.hpp"
#include "head/isIn.hpp"

namespace octolang {
	enum class token_t {
		Error = 0, //returned when an error occured
		FileEnd, //EOF
		Token,
		Operator,
		Keyword,
		Number,
		Double,
		Char, //aka SPECIAL
		String,
		Delimiter,
		NewStatement
	};
	constexpr std::initializer_list<std::string_view> keywords = {
		"function","end",
		"break","continue",
		"for","in","is","while","do",
		"if", "else", "elif", "then"
	};
	constexpr std::initializer_list<char> opchars = {
		'+','-','*','/','^','%','&','|','~','!','<','>','=','.'
	};
	constexpr std::initializer_list<std::string_view> operators = {
		"=","+=","-=", "*=", "/=", "^=", "%=", "&=", "|=", "~=", ">>=", "<<=",
		"<=", ">=", "==", "!=", ">", "<", 
		"||", "&&", "!",
		"|", "&", "^", "~",
		"+","-",
		"*","/",
		"**","%",
	};
	struct token {
		token_t t = token_t::Error;
		union {
			std::string s;
			int64_t i;
			double d;
			char c;
		};
		~token() noexcept {
			switch (t) {
			case token_t::Token:
			case token_t::Operator:
			case token_t::Keyword:
				s.~basic_string();
				break;
			}
		}
		token& setT(token_t t) {
			this->t = t;
			return *this;
		}
		token& setC(char c) {
			this->c = c;
			return *this;
		}
		token& setS(std::string s) {
			this->s = s;
			return *this;
		}
		token& setI(int64_t i) {
			this->i = i;
			return *this;
		}
		token& setD(double d) {
			this->d = d;
			return *this;
		}
		token() {};
		token(token_t ty) : t(ty) {};
		token(token& t) {
			switch (t.t) {
			case token_t::Token:
			case token_t::Operator:
			case token_t::Keyword:
				new (&s) std::string(std::move(t.s));
				break;
			case token_t::Number:
				i = t.i;
				break;
			case token_t::Double:
				d = t.d;
				break;
			case token_t::Char:
				c = t.c;
				break;
			}
			this->t = t.t;
		}
		token& operator=(token&& t) {
			switch (t.t) {
			case token_t::Token:
			case token_t::Operator:
			case token_t::Keyword:
				new (&s) std::string(std::move(t.s));
				break;
			case token_t::Number:
				i = t.i;
				break;
			case token_t::Double:
				d = t.d;
				break;
			case token_t::Char:
				c = t.c;
				break;
			}
			this->t = t.t;
			return *this;
		}
		static token error() {
			return token();
		}
	};
	
	Closeable_Generator_Ptr<char> str_iter(std::string& str) {
		class str_generator {
		public:
			std::string* s;
			std::string::iterator it;
			str_generator(std::string& s) {
				new (this->s) std::string(s);
				it = this->s->begin();
			}
			char next() {
				return *it++;
			}
			bool done() {
				return it == s->end();
			}
			void close() {
				s->~basic_string();
				delete this; //goodbye.
			}
		};
		return to_cgen_ptr<str_generator, char>(new str_generator(str));
	}
	class Lexer { //closeable_generator
	private:
		void set_gen(closeable_generator<char> auto gen) noexcept {
			this->gen = to_gen_ptr<char>(gen);
		}
	protected:
		Closeable_Generator_Ptr<char> gen; //Input generator
		std::string error;
		bool errorflag = false;
		char protectednext() {
			if (char_to_reload != 0) {
				char c = char_to_reload;
				char_to_reload = 0;
				return c;
			}
			else if (!gen.isopen())
				return 0;
			else if (gen.done()) {
				_tryclose();
				return 0;
			}
			else return gen.next();
		}
		void _tryclose() noexcept {
			if (gen.isopen()) {
				gen.close();
				gen = nullptr;
			}
		}
		void seterror(std::string err) noexcept {
			error = err;
			errorflag = true;
			_tryclose(); //free memory if possible
		}
		token serr(std::string err) noexcept {
			seterror(err);
			return token::error();
		}
		void reseterror() noexcept {
			error.clear();
			errorflag = false;
		}
		void reload(char c) noexcept {
			char_to_reload = c;
		}
		//Lexer state ---
		token current;
		char char_to_reload;
		// Lexer state END
		static token& keyword_check(token& t) {
			if (isIn(keywords, t.s)) {
				t.t = token_t::Keyword;
			}
			return t;
		}
		void comment_clear() noexcept {
			for (;;) {
				char c = protectednext();
				if (c == 0) return;
				else if (c == '\n') return;
			}
		}
		void wsp_clear() noexcept {
			for (;;) {
				char c = protectednext();
				if (c == 0) return;
				elif(c == ' ' || c == '\t' || c == '\r' || c == '\n') continue;
				else {
					reload(c);
					return;
				}
			}
		}
		token operator_read() noexcept {
			current.~token();
			new (&current) token(token_t::Operator);
			new (&current.s) std::string();
			for (;;) {
				char c = protectednext();
				if  (isIn(opchars, c)) 
					current.s.push_back(c);
				else
				{
					reload(c); //End of operator, push non-op character back and return token.

					return current;
				}
			}
		}
		void str_escapehex() noexcept {
			char v = 0;
			char i = 0;
			for (;;) {
				char c = protectednext();
				if (c == 0) return;
				if (c >= '0' && c <= '9') {
					v = c - '0';
				}
				else if (c >= 'a' && c <= 'f') {
					v = c - 'a' + 10;
				}
				else if (c >= 'A' && c <= 'F') {
					v = c - 'A' + 10;
				}
				else {
					reload(c);
					return;
				}
				if (i++ == 0) {
					v <<= 4;
				}
				else {
					current.s.push_back(v);
					return;
				}
			}
		}
		void str_escape() noexcept {
			char c = protectednext();
			switch (c) {
			case 'a':
				current.s.push_back('\a');
				break;
			case 'b':
				current.s.push_back('\b');
				break;
			case 'f':
				current.s.push_back('\f');
				break;
			case 'n':
				current.s.push_back('\n');
				break;
			case 'r':
				current.s.push_back('\r');
				break;
			case 't':
				current.s.push_back('\t');
				break;
			case 'v':
				current.s.push_back('\v');
				break;
			case '\\':
				current.s.push_back('\\');
				break;
			case '\'':
				current.s.push_back('\'');
				break;
			case '\"':
				current.s.push_back('\"');
				break;
			case '\n':
			case '\r':
				break;
			case 'z':
				wsp_clear();
				break;
			case 'x':
				str_escapehex();
				break;
			}
		}
		token str_read(char q) noexcept {
			current.~token();
			new (&current) token(token_t::String);
			new (&current.s) std::string();
			for (;;) {
				char c = protectednext();
				if (c == 0) return serr("Unexpected EOF");
				elif(c == q) {
					return current;
				}
				elif(c == '\\') str_escape();
				else current.s.push_back(c);
			}
		}
		token token_read() noexcept {
			new (&current) token(token_t::Token);
			new (&current.s) std::string();
			for (;;) {
				char c = protectednext();
				switch (c) {
				case 0:
				case ' ':case '\t':case '\r':case '\n':
				case '"':case '\'':
				case '#':
				case '+':case '-':case '*':case '/':case '^':case '%':
				case '&':case '|':case '~':case '!':case '<':case '>':
				case '=':case '.':
				case '(':case ')':case '[':case ']':case '{':case '}':
				case ',':case ';':
					reload(c);
					return keyword_check(current);
				default:
					current.s.push_back(c);
					break;
				}
			}
		}
		token tokenloop() noexcept {
			for (;;) {
				char c = protectednext();
				switch (c) {
				case 0:
					return token(token_t::FileEnd);
				case ' ':
				case '\t':
				case '\r':
				case '\n':
					continue; //Whitespace, advance to next character
				case '"':
				case '\'':
					return str_read(c); //Read a string
				case '#':
					comment_clear();
					continue;
				case '+':
				case '-':
				case '*':
				case '/':
				case '%':
				case '^':
				case '&':
				case '|':
				case '~':
				case '!':
				case '=':
				case '<':
				case '>':
				case '.':
					reload(c);
					return operator_read();
				case '(':
				case ')':
				case '[':
				case ']':
				case '{':
				case '}':
					return token(token_t::Char).setC(c); //special character
				case ',':
					return token(token_t::Delimiter).setC(c); //delimiter
				case ';':
					return token(token_t::NewStatement).setC(c); //new statement
				default:
					reload(c);
					return token_read();
				}
			}	
		}

	public:
		void _reset(closeable_generator<char> auto gen) noexcept {
			_tryclose();
			reseterror();
			set_gen(gen);
		}
		Lexer(closeable_generator<char> auto gen) noexcept {
			set_gen(gen);
		}
		void close() {
			_tryclose();
			reseterror();
			current.~token();
			//can now be safely discarded
			delete this;
		}
		token next() noexcept {
			if (errorflag) return token::error(); //has errored
			else if (!gen.isopen()) return token(token_t::FileEnd); //EOF
			else if (gen.done()) { //EOF
				_tryclose();
				return token(token_t::FileEnd);
			}
			else return this->tokenloop(); //newtoken
		}
		bool done() noexcept {
			return !gen.isopen();
		}
	};
};