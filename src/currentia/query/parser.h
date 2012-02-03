// -*- c++ -*-

#ifndef CURRENTIA_QUERY_PARSER_H_
#define CURRENTIA_QUERY_PARSER_H_

#include "currentia/query/lexer.h"
#include "currentia/trait/non-copyable.h"

#include "currentia/query/ast.h"

#define ASSERT_TOKEN(token)                                     \
    do {                                                        \
        if (current_token_ != Lexer::token) {                   \
            report_error_(                                      \
                "Expected " #token ", but got "                 \
                + Lexer::token_to_string(current_token_)        \
            );                                                  \
        }                                                       \
    } while (0)

#define ASSERT_TOKEN_IS_NUMBER()                        \
    do {                                                \
        if (current_token_ != Lexer::INTEGER &&         \
            current_token_ != Lexer::FLOAT) {           \
            report_error_("Expected INTEGER or FLOAT"); \
        }                                               \
    } while (0)

namespace currentia {
    class Parser: private NonCopyable<Parser> {
        Lexer::ptr_t lexer_ptr_;

        Lexer::Token current_token_;
        Lexer::Token get_next_token_() {
            current_token_ = lexer_ptr_->next_token();
            return current_token_;
        }

        const std::string dummy_current_string_;
        const std::string& get_current_string_() {
            switch (current_token_) {
            case Lexer::NAME:
                return lexer_ptr_->get_latest_name();
            case Lexer::INTEGER:
            case Lexer::FLOAT:
                return lexer_ptr_->get_latest_number();
            case Lexer::STRING:
                return lexer_ptr_->get_latest_string();
            default:
                return dummy_current_string_;
            }
        }

    public:
        Parser(Lexer::ptr_t lexer_ptr):
            lexer_ptr_(lexer_ptr),
            current_token_(Lexer::UNKNOWN),
            selection_depth_(0) {
        }

        void parse() {
            try {
                get_next_token_();
                while (current_token_ != Lexer::EOS)
                    parse_statement_();
            } catch (std::string error) {
                std::cerr << "\nSyntax Error: " << error << std::endl;
            }
        }

        AbstractNode::ptr_t parse_statement() {
            try {
                get_next_token_();
                return parse_statement_();
            } catch (std::string error) {
                std::cerr << "\nSyntax Error: " << error << std::endl;
                return AbstractNode::ptr_t();
            }
        }

    private:
        void report_error_(std::string message) {
            std::stringstream ss;
            long line_number = lexer_ptr_->get_current_line_number();
            ss << "Parse error: line " << line_number
               << ": (" << Lexer::token_to_string(current_token_) << "): "
               << message
               << std::endl;
            lexer_ptr_->print_error_point(ss);
            throw ss.str();
        }

        static const char INDENTATION_CHAR = '\t';
        void indented_print(std::string message) {
            for (int i = 1; i < selection_depth_; ++i)
                std::cout << INDENTATION_CHAR;
            std::cout << message << std::endl;
        }

        AbstractNode::ptr_t parse_statement_() {
            switch (current_token_) {
            case Lexer::SELECT:
                return parse_selection_();
            case Lexer::CREATE:
                return parse_create_();
            default:
                report_error_("Expected SELECT or CREATE");
            }
        }

        // ------------------------------------------------------------ //
        // DML
        // ------------------------------------------------------------ //

        // <SELECTION> : SELECT <ATTRIBUTES> FROM <FROM_ELEMENTS> WHERE <CONDITIONS>
        long selection_depth_;
        AbstractNode::ptr_t parse_selection_() {
            selection_depth_++;

            ASSERT_TOKEN(SELECT);
            get_next_token_();

            parse_attributes_();

            ASSERT_TOKEN(FROM);
            get_next_token_();

            parse_from_elements_(); // returns std::list<Operator::ptr_t>

            ASSERT_TOKEN(WHERE);
            get_next_token_();

            parse_conditions_(); // returns Condition::ptr_t

            selection_depth_--;

            return AbstractNode::ptr_t();
        }

        // <ATTRIBUTES> := <ATTRIBUTE> (COMMA <ATTRIBUTES>)?
        void parse_attributes_() {
            parse_attribute_();

            if (current_token_ == Lexer::COMMA) {
                get_next_token_(); // Trash COMMA
                parse_attributes_();
            }
        }

        // <ATTRIBUTE> := NAME (DOT NAME)?
        std::string parse_attribute_() {
            ASSERT_TOKEN(NAME);

            std::string stream_name = "DEFAULT_STREAM";
            std::string attribute_name = get_current_string_();

            if (get_next_token_() == Lexer::DOT) { // Trash NAME
                if (get_next_token_() != Lexer::NAME) // Trash DOT
                    report_error_("Attribute name is missing");
                stream_name = attribute_name;
                attribute_name = get_current_string_();
                get_next_token_(); // Trash NAME
            }

            return attribute_name;
        }

        // <FROM_ELEMENTS> := <FROM_ELEMENT> (COMMA <ATTRIBUTES>)?
        void parse_from_elements_() {
            parse_from_element_();

            if (current_token_ == Lexer::COMMA) {
                get_next_token_(); // Trash COMMA
                parse_from_elements_();
            }
        }

        // <FROM_ELEMENT> := (LPAREN <SELECTION> RPAREN | NAME) <WINDOW>?
        void parse_from_element_() {
            switch (current_token_) {
            case Lexer::LPAREN:
                get_next_token_(); // Trash LPAREN
                parse_selection_();
                if (current_token_ != Lexer::RPAREN)
                    report_error_("Unclosed sub-query");
                get_next_token_(); // Trash RPAREN
                break;
            case Lexer::NAME:
                indented_print("Target Stream => " + get_current_string_());
                get_next_token_();  // Trash NAME
                break;
            default:
                report_error_("Expected sub-query or stream name");
                break;
            }

            if (current_token_ == Lexer::LBRACKET)
                parse_window_();

            return SelectionNode::ptr_t();
        }

        // <WINDOW> := LBRACKET
        //                 (INTEGER|FLOAT) (ROWS|MSEC|SEC|MIN|HOUR)?
        //                 (ADVANCE (INTEGER|FLOAT) (ROWS|MSEC|SEC|MIN|HOUR))?
        //             RBRACKET
        void parse_window_() {
            ASSERT_TOKEN(LBRACKET);
            get_next_token_(); // Trash LBRACKET

            // parse window width
            ASSERT_TOKEN_IS_NUMBER();
            get_next_token_();  // Trash number

            if (current_token_ == Lexer::ROWS) {
                get_next_token_(); // Trash window width unit
            }

            if (current_token_ == Lexer::ADVANCE) {
                get_next_token_(); // Trash ADVANCE

                // parse window stride
                ASSERT_TOKEN_IS_NUMBER();
                get_next_token_();  // Trash number

                if (current_token_ == Lexer::ROWS) {
                    get_next_token_(); // Trash window stride unit
                }
            }

            ASSERT_TOKEN(RBRACKET);
            get_next_token_(); // Trash RBRACKET
        }

        // <CONDITIONS> := <CONDITION> ((AND | OR) <CONDITIONS>)?
        void parse_conditions_() {
            parse_condition_();

            if (Lexer::is_token_conjunctive(current_token_)) {
                get_next_token_(); // Trash CONJUNCTIVE
                parse_conditions_();
            }
        }

        // <CONDITION> := NOT? (LPAREN <CONDITIONS> RPAREN | <COMPARISON>)
        void parse_condition_() {
            if (current_token_ == Lexer::NOT) {
                get_next_token_(); // Trash NOT
            }

            switch (current_token_) {
            case Lexer::LPAREN:
                get_next_token_(); // Trash LPAREN
                parse_conditions_();
                ASSERT_TOKEN(RPAREN);
                get_next_token_(); // Trash RPAREN
                break;
            default:
                parse_comparison_();
                break;
            }
        }

        // <COMPARISON> := <VALUE> <COMPARATOR> <VALUE>
        void parse_comparison_() {
            parse_value_();
            parse_comparator_();
            parse_value_();
        }

        // <VALUE> := STRING | INTEGER | FLOAT | <ATTRIBUTE>
        void parse_value_() {
            switch (current_token_) {
            case Lexer::STRING:
            case Lexer::INTEGER:
            case Lexer::FLOAT:
                get_next_token_(); // Trash <VALUE>
                break;
            default:
                parse_attribute_();
            }
        }

        // <COMPARATOR> := EQOAL | NOT_EQUAL | LESS_THAN | LESS_THAN_EQUAL | GREATER_THAN | GREATER_THAN_EQUAL
        void parse_comparator_() {
            switch (current_token_) {
            case Lexer::EQUAL:
            case Lexer::NOT_EQUAL:
            case Lexer::LESS_THAN:
            case Lexer::LESS_THAN_EQUAL:
            case Lexer::GREATER_THAN:
            case Lexer::GREATER_THAN_EQUAL:
                get_next_token_(); // Trash <COMPARATOR>
                break;
            default:
                report_error_("Expected <COMPARATOR>");
            }
        }

        // ------------------------------------------------------------ //
        // DDL
        // ------------------------------------------------------------ //

        // <CREATE> := CREATE STREAM NAME (COMMA <ATTRIBUTE_DEFS>)?
        AbstractNode::ptr_t parse_create_() {
            ASSERT_TOKEN(CREATE);
            get_next_token_();  // Trash CREATE

            // TODO: support other create types (TABLE, VIEW, TRIGGER, INDEX, ...)
            ASSERT_TOKEN(STREAM);
            get_next_token_();  // Trash STREAM

            ASSERT_TOKEN(NAME);
            std::string stream_name = get_current_string_();
            get_next_token_();  // Trash NAME

            ASSERT_TOKEN(LPAREN);
            get_next_token_();  // Trash LPAREN

            std::list<Attribute> attributes = parse_attribute_defs_();

            ASSERT_TOKEN(RPAREN);
            get_next_token_();  // Trash RPAREN

            return AbstractNode::ptr_t(new CreateNode(stream_name, attributes));
        }

        // <ATTRIBUTE_DEFS> := <ATTRIBUTE_DEF> (COMMA <ATTRIBUTE_DEFS>)?
        std::list<Attribute> parse_attribute_defs_() {
            std::list<Attribute> attributes;
            attributes.push_back(parse_attribute_def_());

            if (current_token_ == Lexer::COMMA) {
                get_next_token_(); // Trash COMMA
                std::list<Attribute> rest_attributes = parse_attribute_defs_();
                attributes.insert(attributes.end(), rest_attributes.begin(), rest_attributes.end());
            }

            return attributes;
        }

        // <ATTRIBUTE_DEF> := <NAME> <NAME>
        Attribute parse_attribute_def_() {
            ASSERT_TOKEN(NAME);
            std::string attribute_name = get_current_string_();
            get_next_token_();

            ASSERT_TOKEN(NAME);
            std::string type_name = get_current_string_();
            get_next_token_();

            Object::Type type = Object::string_to_type(type_name);
            if (type == Object::UNKNOWN)
                report_error_("Unknown type " + type_name);

            return Attribute(attribute_name, type);
        }
    };
}

#undef ASSERT_TOKEN

#endif  /* ! CURRENTIA_QUERY_PARSER_H_  */
