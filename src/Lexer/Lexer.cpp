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
		node->Boolean.value = true;
	}

	else if (name == "false")
	{
		node->type = NodeType::BOOLEAN;
		node->Boolean.value = false;
	}
	else
	{
		node->ID.value = name;
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
		node->Number.value = std::stol(value);
	}
	else if (num_dots == 1)
	{
		node->type = NodeType::NUMBER;
		node->Number.value = std::stod(value);
	}
	else
	{
		error_and_exit("Unacceptable number of dots in number node.");
	}

	nodes.push_back(node);
}

void Lexer::build_string()
{
	node_ptr node(new Node(NodeType::STRING, line, column));

	std::string str = std::string();

	advance();

	while (current_char != '"')
	{
		if (current_char == '\0')
		{
			error_and_exit("Warning: Missing end '\"', end of file reached.");
			return;
		}

		if (current_char == '\n')
		{
			line++;
			column = 0;
			advance(); // consume '\n'
		}

		if (current_char == '\\' && peek() == '"')
		{
			str += current_char;
			advance();
			str += current_char;
			advance();
		}
		else
		{
			str += current_char;
			advance();
		}

	}

	advance();

	node->String.value = std::string();

	for (int i = 0; i < (str).length(); i++)
	{
		if ((str)[i] == '\\' && (str)[i + 1] == 'n')
		{
			(str)[i] = '\n';
			(node->String.value).push_back('\n');
			i++;
		}
		else if ((str)[i] == '\\' && (str)[i + 1] == 'r')
		{
			(str)[i] = '\r';
			(node->String.value).push_back('\r');
			i++;
		}
		else if ((str)[i] == '\\' && (str)[i + 1] == 't')
		{
			(str)[i] = '\t';
			(node->String.value).push_back('\t');
			i++;
		}
		else if ((str)[i] == '\\' && (str)[i + 1] == '"')
		{
			(str)[i] = '\"';
			(node->String.value).push_back('\"');
			i++;
		}
		else
		{
			(node->String.value).push_back((str)[i]);
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
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = "...";
			nodes.push_back(node);
			advance(); // consume .
			advance(); // consume .
			advance(); // consume .
		}
		else if (current_char == '.' && peek() == '.')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = "..";
			nodes.push_back(node);
			advance(); // consume .
			advance(); // consume .
		}
		else if (current_char == '=' && peek() == '=')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = "==";
			nodes.push_back(node);
			advance(); // consume =
			advance(); // consume =
		}
		else if (current_char == '!' && peek() == '=')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = "!=";
			nodes.push_back(node);
			advance(); // consume !
			advance(); // consume =
		}
		else if (current_char == '<' && peek() == '=')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = "<=";
			nodes.push_back(node);
			advance(); // consume <
			advance(); // consume =
		}
		else if (current_char == '>' && peek() == '=')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = ">=";
			nodes.push_back(node);
			advance(); // consume >
			advance(); // consume =
		}
		else if (current_char == '+' && peek() == '=')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = "+=";
			nodes.push_back(node);
			advance(); // consume +
			advance(); // consume =
		}
		else if (current_char == '-' && peek() == '=')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = "-=";
			nodes.push_back(node);
			advance(); // consume -
			advance(); // consume =
		}
		else if (current_char == '=' && peek() == '>')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = "=>";
			nodes.push_back(node);
			advance(); // consume =
			advance(); // consume >
		}
		else if (current_char == '-' && peek() == '>')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = "->";
			nodes.push_back(node);
			advance(); // consume >
			advance(); // consume >
		}
		else if (current_char == '>' && peek() == '>')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = ">>";
			nodes.push_back(node);
			advance(); // consume >
			advance(); // consume >
		}
		else if (current_char == '&' && peek() == '&')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = "&&";
			nodes.push_back(node);
			advance(); // consume &
			advance(); // consume &
		}
		else if (current_char == '|' && peek() == '|')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = "||";
			nodes.push_back(node);
			advance(); // consume |
			advance(); // consume |
		}
		else if (current_char == ':' && peek() == ':')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = "::";
			nodes.push_back(node);
			advance(); // consume :
			advance(); // consume :
		}
		else if (current_char == '=')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '(')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == ')')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '{')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '}')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '[')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == ']')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '<')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '>')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '.')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '\\')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '\'')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '!')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '@')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '#')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '$')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '^')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '?')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '%')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '"')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '-')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '+')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '/')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '*')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == ',')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == '|')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == ':')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = current_char;
			nodes.push_back(node);
			advance(); // consume symbol
		}
		else if (current_char == ';')
		{
			node_ptr node(new Node(NodeType::OP, line, column));
			node->Operator.value = current_char;
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