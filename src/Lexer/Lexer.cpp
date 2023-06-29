#include "Lexer.hpp"

void Lexer::load_source(std::string filename)
{
	std::ifstream stream(filename);
	std::string source_str((std::istreambuf_iterator<char>(stream)),
		std::istreambuf_iterator<char>());
	source = source_str;
}

void Lexer::error_and_exit(std::string message)
{
	std::string error_message = "\n\n[Lexer] Lexical Error in '" + file_name + "' @ (" + std::to_string(line) + ", " + std::to_string(column) + "): " + message;
	std::cout << error_message;
	std::cout << "\nCompilation failed. Press any key to exit...";
	std::cin.get();
	exit(1);
}

void Lexer::advance()
{
	index++;
	column++;
	if (index >= source_length)
	{
		current_char = '\0';
	}
	else
	{
		current_char = source[index];
	}
}

char Lexer::peek(int n)
{
	int peek_index = index + n;
	if (peek_index >= source_length)
	{
		return '\0';
	}
	else
	{
		return source[peek_index];
	}
}

void Lexer::build_identifier()
{
	node_ptr node(new Node(NodeType::ID, line, column));

	std::string name = std::string();

	while (isalpha(current_char) || current_char == '_' || isdigit(current_char))
	{
		name += current_char;
		advance();
	}

	if (name == "true")
	{
		node->type = NodeType::BOOLEAN;
		node->_Node = BooleanNode();
		node->_Node.Boolean().value = true;
	}
	else if (name == "false")
	{
		node->type = NodeType::BOOLEAN;
		node->_Node = BooleanNode();
		node->_Node.Boolean().value = false;
	}
	else if (name == "as") {
		node->type = NodeType::OP;
		node->_Node = OpNode();
		node->_Node.Op().value = name;
	}
	else if (name == "is") {
		node->type = NodeType::OP;
		node->_Node = OpNode();
		node->_Node.Op().value = name;
	}
	else if (name == "in") {
		node->type = NodeType::OP;
		node->_Node = OpNode();
		node->_Node.Op().value = name;
	}
	else if (name == "None")
	{
		node->type = NodeType::NONE;
	}
	else
	{
		node->_Node.ID().value = name;
	}

	nodes.push_back(node);
}

void Lexer::build_number()
{
	node_ptr node(new Node(line, column));

	std::string value;
	int num_dots = 0;

	while (isdigit(current_char) || current_char == '.')
	{
		value += current_char;

		if (current_char == '.')
		{
			num_dots++;
		}

		advance();

		if (current_char == '.' && peek(1) == '.' && peek(2) == '.')
		{
			break;
		}

		if (current_char == '.' && peek(1) == '.')
		{
			break;
		}

	}

	if (num_dots == 0)
	{
		node->type = NodeType::NUMBER;
		node->_Node = NumberNode();
		node->_Node.Number().value = std::stol(value);
	}
	else if (num_dots == 1)
	{
		node->type = NodeType::NUMBER;
		node->_Node = NumberNode();
		node->_Node.Number().value = std::stod(value);
	}
	else
	{
		error_and_exit("Unacceptable number of dots in number node.");
	}

	nodes.push_back(node);
}

void Lexer::format_string()
{
	int current_index = index;

	source.erase(source.begin() + index);
	current_char = source[index];
	advance();

	while (current_char != '"') {
		if (current_char == '\\' && peek() == '"')
		{
			advance();
			advance();
			continue;
		}
		if (current_char == '$' && peek() == '{') {
			source[index] = '"';
			advance();
			source[index] = '+';
			advance();
			source.insert(index, "string(");
			source_length += 7;
			index += 7;
			current_char = source[index];
			int num_brackets = 1;
			while (true) {
				if (current_char == '\0') {
					error_and_exit("Malformed string interpolation - missing '}'");
				}
				if (current_char == '{') {
					num_brackets++;
				}
				if (current_char == '}') {
					num_brackets--;
				}
				if (current_char == '}' && num_brackets == 0) {
					break;
				}
				advance();
			}
			source[index] = ')';
			advance();
			source.insert(index, "+\"");
			source_length += 2;
			current_char = source[index];
			advance();
		}
		advance();
	}

	index = current_index;
	current_char = source[index];
}

void Lexer::build_string()
{
	node_ptr node(new Node(NodeType::STRING, line, column));

	std::string str = std::string();

	advance();

	while (true)
	{
		if (current_char == '\0')
		{
			error_and_exit("Warning: Missing end '\"', end of file reached.");
			return;
		}

		if (current_char == '\\' && peek() == '\\') {
			str += current_char;
			advance();
			str += current_char;
			advance();
		} else if (current_char == '\\' && peek() == '"') {
			str += current_char;
			advance();
			str += current_char;
			advance();
		} else if (current_char == '\n') {
			line++;
			column = 0;
			advance(); // consume '\n'
		} else if (current_char == '"') {
			if (peek(-1) != '\\') {
				break;
			} else if (peek(-1) == '\\' && peek(-2) == '\\') {
				break;
			}
		}
		else {
			str += current_char;
			advance();
		}
	}

	advance();

	node->_Node.String().value = std::string();

	for (int i = 0; i < (str).length(); i++)
	{
		if ((str)[i] == '\\' && (str)[i + 1] == 'n')
		{
			(str)[i] = '\n';
			(node->_Node.String().value).push_back('\n');
			i++;
		}
		else if ((str)[i] == '\\' && (str)[i + 1] == 'r')
		{
			(str)[i] = '\r';
			(node->_Node.String().value).push_back('\r');
			i++;
		}
		else if ((str)[i] == '\\' && (str)[i + 1] == 't')
		{
			(str)[i] = '\t';
			(node->_Node.String().value).push_back('\t');
			i++;
		}
		else if ((str)[i] == '\\' && (str)[i + 1] == '"')
		{
			(str)[i] = '\"';
			(node->_Node.String().value).push_back('\"');
			i++;
		}
		else if ((str)[i] == '\\' && (str)[i + 1] == '\'')
		{
			(str)[i] = '\'';
			(node->_Node.String().value).push_back('\'');
			i++;
		}
		else if ((str)[i] == '\\' && isdigit((str)[i + 1]) && isdigit((str)[i + 2]) && isdigit((str)[i + 3]))
		{	
			std::string escapeCode = {(str)[i + 1], (str)[i + 2], (str)[i + 3]};
			char escapeChar = static_cast<char>(std::stoi(escapeCode, nullptr, 8));

			(str)[i] = escapeChar;
			(node->_Node.String().value).push_back((str)[i]);
			i++;
			i++;
			i++;
		}
		else if ((str)[i] == '\\' && ((str)[i + 1] == 'O' || (str)[i + 1] == 'o'))
		{	
			try {
				std::string escapeCode = {(str)[i + 2], (str)[i + 3], (str)[i + 4]};
				char escapeChar = static_cast<char>(std::stoi(escapeCode, nullptr, 8));

				(str)[i] = escapeChar;
				(node->_Node.String().value).push_back((str)[i]);
				i++;
				i++;
				i++;
			} catch (...) {}
		}
		else if ((str)[i] == '\\' && ((str)[i + 1] == 'X' || (str)[i + 1] == 'x'))
		{	
			try {
				// 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E and F

				std::string escapeCode = "";
				int hex_i = i;
				hex_i++;
				hex_i++;
				char hex_c = str[hex_i];
				while (isdigit(hex_c) || hex_c == 'A' || hex_c == 'B' || hex_c == 'C' || hex_c == 'D' || hex_c == 'E' || hex_c == 'F' || hex_c == 'a' || hex_c == 'b' || hex_c == 'c' || hex_c == 'd' || hex_c == 'e' || hex_c == 'f') {
					escapeCode.push_back(hex_c);
					hex_i++;
					hex_c = str[hex_i];
				}

				char escapeChar = static_cast<char>(std::stoi(escapeCode, nullptr, 16));

				(str)[i] = escapeChar;
				(node->_Node.String().value).push_back((str)[i]);
				for (int it = 0; it <= escapeCode.size(); it++) {
					i++;
				}
			} catch (...) {}
		}
		else if ((str)[i] == '\\' && ((str)[i + 1] == 'U' || (str)[i + 1] == 'u'))
		{	
			std::string escapeCode = {(str)[i + 2], (str)[i + 3], (str)[i + 4], (str)[i + 5]};

			unsigned int codepoint = std::stoul(escapeCode, nullptr, 16);
			std::string utf8Character;

			if (codepoint <= 0x7F) {
        		utf8Character += static_cast<char>(codepoint);
			}
			else if (codepoint <= 0x7FF) {
				utf8Character += static_cast<char>(0xC0 | ((codepoint >> 6) & 0x1F));
				utf8Character += static_cast<char>(0x80 | (codepoint & 0x3F));
			}
			else if (codepoint <= 0xFFFF) {
				utf8Character += static_cast<char>(0xE0 | ((codepoint >> 12) & 0x0F));
				utf8Character += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
				utf8Character += static_cast<char>(0x80 | (codepoint & 0x3F));
			}
			else if (codepoint <= 0x10FFFF) {
				utf8Character += static_cast<char>(0xF0 | ((codepoint >> 18) & 0x07));
				utf8Character += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
				utf8Character += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
				utf8Character += static_cast<char>(0x80 | (codepoint & 0x3F));
			}

			str.replace(i, 6, utf8Character);
			(node->_Node.String().value).append(utf8Character);
			i++;
		}
		else if ((str)[i] == '\\' && str[i+1] == '\\')
		{
			(str)[i] = '\\';
			(node->_Node.String().value).push_back('\\');
			i++;
		}
		else
		{
			(node->_Node.String().value).push_back((str)[i]);
		}
	}

	nodes.push_back(node);
}

void Lexer::handle_line_comment()
{
	advance();
	advance();

	while (current_char != '\n')
	{
		if (current_char == '\0')
		{
			return;
		}

		advance();
	}

	line++;
	column = 0;

	advance(); // consume '\n'

	return;
}

void Lexer::handle_block_comment()
{
	advance(); // consume '/'
	advance(); // consume '*'

	while (!(current_char == '*' && peek() == '/'))
	{
		if (current_char == '\n')
		{
			line++;
			column = 0;
		}
		if (current_char == '\0')
		{
			error_and_exit("Warning: No end to block comment, end of file reached.");
		}
		if (current_char == '/' && peek() == '*')
		{
			handle_block_comment();
		}

		advance();
	}

	advance(); // consume '*'
	advance(); // consume '/'

	return;
}

void Lexer::init(std::string src)
{
	source = src;
	file_name = "stdin";

	source_length = source.length();
	index = 0;
	current_char = source[0];
	line = 1;
	column = 1;
}

void Lexer::tokenize()
{
	node_ptr SOF(new Node(NodeType::START_OF_FILE, line, column));
	nodes.push_back(SOF);

	while (current_char != '\0')
	{
		if (current_char == '\n')
		{
			column = 0;
			line++;
			advance();
		}
		else if (current_char == 'f' && peek() == '"')
		{
			format_string();
		}
		else if (current_char == ' ' || current_char == '\t')
		{
			advance();
		}
		else if (isalpha(current_char) || current_char == '_')
		{
			build_identifier();
		}
		else if (isdigit(current_char))
		{
			build_number();
		}
		else if (current_char == '/' && peek() == '/')
		{
			handle_line_comment();
		}
		else if (current_char == '/' && peek() == '*')
		{
			handle_block_comment();
		}
		else if (current_char == '"')
		{
			build_string();
		}
		else if (current_char == '.' && peek() == '.' && peek(2) == '.')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = "...";
			nodes.push_back(node);
			advance(); // consume .
			advance(); // consume .
			advance(); // consume .
		}
		else if (current_char == '?' && peek() == '?')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = "??";
			nodes.push_back(node);
			advance(); // consume .
			advance(); // consume .
		}
		else if (current_char == '.' && peek() == '.')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = "..";
			nodes.push_back(node);
			advance(); // consume .
			advance(); // consume .
		}
		else if (current_char == '=' && peek() == '=')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = "==";
			nodes.push_back(node);
			advance(); // consume =
			advance(); // consume =
		}
		else if (current_char == '!' && peek() == '=')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = "!=";
			nodes.push_back(node);
			advance(); // consume !
			advance(); // consume =
		}
		else if (current_char == '<' && peek() == '=')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = "<=";
			nodes.push_back(node);
			advance(); // consume <
			advance(); // consume =
		}
		else if (current_char == '>' && peek() == '=')
		{
			//node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = ">=";
			nodes.push_back(node);
			advance(); // consume >
			advance(); // consume =
		}
		else if (current_char == '+' && peek() == '=')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = "+=";
			nodes.push_back(node);
			advance(); // consume +
			advance(); // consume =
		}
		else if (current_char == '-' && peek() == '=')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = "-=";
			nodes.push_back(node);
			advance(); // consume -
			advance(); // consume =
		}
		else if (current_char == '=' && peek() == '>')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = "=>";
			nodes.push_back(node);
			advance(); // consume =
			advance(); // consume >
		}
		else if (current_char == '-' && peek() == '>')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = "->";
			nodes.push_back(node);
			advance(); // consume >
			advance(); // consume >
		}
		else if (current_char == '>' && peek() == '>')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = ">>";
			nodes.push_back(node);
			advance(); // consume >
			advance(); // consume >
		}
		else if (current_char == '&' && peek() == '&')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = "&&";
			nodes.push_back(node);
			advance(); // consume &
			advance(); // consume &
		}
		else if (current_char == '|' && peek() == '|')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = "||";
			nodes.push_back(node);
			advance(); // consume |
			advance(); // consume |
		}
		else if (current_char == ':' && peek() == ':')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = "::";
			nodes.push_back(node);
			advance(); // consume :
			advance(); // consume :
		}
		else if (current_char == '=')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '(')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == ')')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '{')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '}')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '[')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == ']')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '<')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '>')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '.')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '\\')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '\'')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '!')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '@')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '#')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '$')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '^')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '?')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '%')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '"')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '-')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '+')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '/')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '*')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == ',')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '|')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == ':')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == ';')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '&')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '@')
		{
			node_ptr node = std::make_shared<Node>(NodeType::OP, line, column);
			node->_Node.Op().value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else
		{
			error_and_exit("Unexpected token '" + std::string(1, current_char) + "'.");
		}
	}

	node_ptr node(new Node(NodeType::END_OF_FILE, line, column));
	nodes.push_back(node);
}