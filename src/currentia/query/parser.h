// -*- c++ -*-

#ifndef CURRENTIA_QUERY_PARSER_H_
#define CURRENTIA_QUERY_PARSER_H_

#include "currentia/query/lexer.h"

namespace currentia {
    class Parser {
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
                while (get_next_token_() != Lexer::EOF) {
                    parse_statement_();
                }
            } catch (std::string error) {
                std::cerr << "\nSyntax Error: " << error << std::endl;
            }
        }

    private:
        void report_error_(std::string message) {
            std::stringstream ss;
            long line_number = lexer_ptr_->get_current_line_number();
            ss << "Parse error: line " << line_number
               << ": (" << Lexer::token_to_string(current_token_) << "): "
               << message;
            throw ss.str();
        }

        static const char INDENTATION_CHAR = '\t';
        void indented_print(std::string message) {
            for (int i = 1; i < selection_depth_; ++i)
                std::cout << INDENTATION_CHAR;
            std::cout << message << std::endl;
        }

        void parse_statement_() {
            switch (current_token_) {
            case Lexer::SELECT:
                parse_selection_();
                break;
            case Lexer::CREATE:
                report_error_("DDL is unimplemented");
                break;
            default:
                report_error_("Expected SELECT or CREATE");
            }
        }

        // <SELECTION> : SELECT <ATTRIBUTES> FROM <FROM_ELEMENTS> WHERE <CONDITIONS>
        long selection_depth_;
        void parse_selection_() {
            selection_depth_++;

            if (current_token_ != Lexer::SELECT)
                report_error_("Expected SELECT");
            get_next_token_();

            parse_attributes_();

            if (current_token_ != Lexer::FROM)
                report_error_("Expected FROM");
            get_next_token_();

            parse_from_elements_();

            if (current_token_ != Lexer::WHERE)
                report_error_("Expected WHERE");
            get_next_token_();

            parse_conditions_();

            selection_depth_--;
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
        void parse_attribute_() {
            if (current_token_ != Lexer::NAME)
                report_error_("Expected NAME");

            std::string stream_name = "DEFAULT_STREAM";
            std::string attribute_name = get_current_string_();

            if (get_next_token_() == Lexer::DOT) { // Trash NAME
                if (get_next_token_() != Lexer::NAME) // Trash DOT
                    report_error_("Attribute name is missing");
                stream_name = attribute_name;
                attribute_name = get_current_string_();
                get_next_token_(); // Trash NAME
            }

            indented_print("Attribute(" + stream_name + ", " + attribute_name + ")");
        }

        // <FROM_ELEMENTS> := <FROM_ELEMENT> (COMMA <ATTRIBUTES>)?
        void parse_from_elements_() {
            parse_from_element_();

            if (current_token_ == Lexer::COMMA) {
                get_next_token_(); // Trash COMMA
                parse_from_elements_();
            }
        }

        // <FROM_ELEMENT> := (LPAREN <SELECTION> RPAREN | NAME)
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
        }

        // <CONDITIONS> := <CONDITION> ((AND | OR) <CONDITIONS>)?
        void parse_conditions_() {
            parse_condition_();

            if (Lexer::is_token_conjunctive(current_token_)) {
                get_next_token_(); // Trash CONJUNCTIVE
                parse_conditions_();
            }
        }

        // <CONDITION> := (LPAREN <CONDITIONS> RPAREN | <COMPARISON>)
        void parse_condition_() {
            switch (current_token_) {
            case Lexer::LPAREN:
                get_next_token_(); // Trash LPAREN
                parse_conditions_();
                if (current_token_ != Lexer::RPAREN)
                    report_error_("Expected ')'");
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
    };
}

#endif  /* ! CURRENTIA_QUERY_PARSER_H_  */
