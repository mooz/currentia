// -*- c++ -*-

#ifndef CURRENTIA_QUERY_PARSER_H_
#define CURRENTIA_QUERY_PARSER_H_

#include "currentia/query/lexer.h"

#include "currentia/query/ast/statement.h"

#include "currentia/query/ast/create.h"
#include "currentia/query/ast/from-selection.h"

#include "currentia/trait/non-copyable.h"
#include "currentia/trait/pointable.h"

#include "currentia/core/operator/condition.h"

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
        if (current_token_ != Lexer::TOKEN_INTEGER &&         \
            current_token_ != Lexer::TOKEN_FLOAT) {           \
            report_error_("Expected INTEGER or FLOAT"); \
        }                                               \
    } while (0)

namespace currentia {
    class Parser: private NonCopyable<Parser>,
                  public Pointable<Parser> {
        Lexer::ptr_t lexer_ptr_;

        Lexer::Token current_token_;
        Lexer::Token get_next_token_() {
            current_token_ = lexer_ptr_->get_next_token();
            return current_token_;
        }

        std::string get_current_string_() {
            switch (current_token_) {
            case Lexer::TOKEN_NAME:
            case Lexer::TOKEN_INTEGER:
            case Lexer::TOKEN_FLOAT:
                return lexer_ptr_->get_token_text();
            default:
                return "";
            }
        }

        double get_current_number_() {
            double current_number;
            std::istringstream(get_current_string_()) >> current_number;
            return current_number;
        }

    public:
        Parser(Lexer::ptr_t lexer_ptr):
            lexer_ptr_(lexer_ptr),
            current_token_(Lexer::TOKEN_UNKNOWN),
            selection_depth_(0) {
        }

        void parse() {
            try {
                get_next_token_();
                while (current_token_ != Lexer::TOKEN_EOS)
                    parse_statement_();
            } catch (std::string error) {
                std::cerr << "\nSyntax Error: " << error << std::endl;
            }
        }

        Statement::ptr_t parse_statement() {
            try {
                get_next_token_();
                return parse_statement_();
            } catch (std::string error) {
                std::cerr << "\nSyntax Error: " << error << std::endl;
                return Statement::ptr_t();
            }
        }

        static Parser::ptr_t create_parser_from_stream(std::istream& input) {
            Lexer::ptr_t lexer(new Lexer(&input));
            Parser::ptr_t parser(new Parser(lexer));

            return parser;
        }

        static Statement::ptr_t parse_statement_from_stream(std::istream& input) {
            return create_parser_from_stream(input)->parse_statement();
        }

        static Condition::ptr_t parse_conditions_from_stream(std::istream& input) {
            // for internal parsing methods, we have to call get_next_token_() at first.
            Parser::ptr_t parser = create_parser_from_stream(input);
            parser->get_next_token_();
            return parser->parse_conditions_();
        }

        static CreateNode::ptr_t parse_create_from_stream(std::istream& input) {
            // same reason as mentioned in parse_conditions_from_stream()
            Parser::ptr_t parser = create_parser_from_stream(input);
            parser->get_next_token_();
            return parser->parse_create_();
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
        void print_indent() {
            for (int i = 1; i < selection_depth_; ++i)
                std::cout << INDENTATION_CHAR;
        }

        void indented_print(const std::string& message) {
            print_indent();
            std::cout << message << std::endl;
        }

        Statement::ptr_t parse_statement_() {
            switch (current_token_) {
            case Lexer::TOKEN_SELECT:
                return parse_selection_();
            case Lexer::TOKEN_CREATE:
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
        Statement::ptr_t parse_selection_() {
            // FromSelectionNode::ptr_t selection_node(new FromSelectionNode());

            selection_depth_++;

            ASSERT_TOKEN(TOKEN_SELECT);
            get_next_token_();

            parse_attributes_();

            ASSERT_TOKEN(TOKEN_FROM);
            get_next_token_();

            parse_from_elements_(); // returns std::list<Operator::ptr_t>

            ASSERT_TOKEN(TOKEN_WHERE);
            get_next_token_();

            parse_conditions_(); // returns Condition::ptr_t

            selection_depth_--;

            return Statement::ptr_t();
        }

        // <ATTRIBUTES> := <ATTRIBUTE> (COMMA <ATTRIBUTES>)?
        std::list<AttributeIdentifier> parse_attributes_() {
            std::list<AttributeIdentifier> attribute_identifiers;

            AttributeIdentifier attribute_identifier = parse_attribute_();
            attribute_identifiers.push_back(attribute_identifier);

            if (current_token_ == Lexer::TOKEN_COMMA) {
                get_next_token_(); // Trash COMMA
                std::list<AttributeIdentifier> rest_attribute_identifiers = parse_attributes_();
                attribute_identifiers.insert(
                    attribute_identifiers.end(),
                    rest_attribute_identifiers.begin(),
                    rest_attribute_identifiers.end()
                );
            }

            return attribute_identifiers;
        }

        // <ATTRIBUTE> := NAME (DOT NAME)?
        AttributeIdentifier parse_attribute_() {
            ASSERT_TOKEN(TOKEN_NAME);

            std::string stream_name = "DEFAULT_STREAM";
            std::string attribute_name = get_current_string_();

            if (get_next_token_() == Lexer::TOKEN_DOT) { // Trash NAME
                if (get_next_token_() != Lexer::TOKEN_NAME) // Trash DOT
                    report_error_("Attribute name is missing");
                stream_name = attribute_name;
                attribute_name = get_current_string_();
                get_next_token_(); // Trash NAME
            }

            return AttributeIdentifier(attribute_name, stream_name);
        }

        // <FROM_ELEMENTS> := <FROM_ELEMENT> (COMMA <ATTRIBUTES>)?
        void parse_from_elements_() {
            std::list<SelectionNode::ptr_t> from_elements;
            from_elements.push_back(parse_from_element_());

            if (current_token_ == Lexer::TOKEN_COMMA) {
                get_next_token_(); // Trash COMMA
                parse_from_elements_();
            }
        }

        // <FROM_ELEMENT> := (LPAREN <SELECTION> RPAREN | NAME) <WINDOW>?
        SelectionNode::ptr_t parse_from_element_() {
            SelectionNode::ptr_t selection_node(
                new SelectionNode()
            );

            switch (current_token_) {
            case Lexer::TOKEN_LPAREN:
                get_next_token_(); // Trash LPAREN
                parse_selection_();
                if (current_token_ != Lexer::TOKEN_RPAREN)
                    report_error_("Unclosed sub-query");
                get_next_token_(); // Trash RPAREN
                break;
            case Lexer::TOKEN_NAME:
                indented_print("Target Stream => " + get_current_string_());
                get_next_token_();  // Trash NAME
                break;
            default:
                report_error_("Expected sub-query or stream name");
                break;
            }

            if (current_token_ == Lexer::TOKEN_LBRACKET)
                parse_window_();

            return SelectionNode::ptr_t();
        }

        // <WINDOW> := LBRACKET
        //                 (INTEGER|FLOAT) (ROWS|MSEC|SEC|MIN|HOUR|DAY)?
        //                 (ADVANCE (INTEGER|FLOAT) (ROWS|MSEC|SEC|MIN|HOUR|DAY))?
        //             RBRACKET
        void parse_window_() {
            long width;
            long stride;
            Window::Type type = Window::TUPLE_BASE;

            ASSERT_TOKEN(TOKEN_LBRACKET);
            get_next_token_(); // Trash LBRACKET

            // parse window width
            ASSERT_TOKEN_IS_NUMBER();
            width = static_cast<long>(get_current_number_());
            get_next_token_();  // Trash number

            switch (current_token_) {
            case Lexer::TOKEN_ROWS:
                get_next_token_(); // Trash window width unit
                break;
            case Lexer::TOKEN_MSEC:
            case Lexer::TOKEN_SEC:
            case Lexer::TOKEN_MIN:
            case Lexer::TOKEN_HOUR:
            case Lexer::TOKEN_DAY:
                get_next_token_(); // Trash window width unit
                break;
            default:
                break;
            }

            if (current_token_ == Lexer::TOKEN_ADVANCE) {
                get_next_token_(); // Trash ADVANCE

                // parse window stride
                ASSERT_TOKEN_IS_NUMBER();
                get_next_token_();  // Trash number

                switch (current_token_) {
                case Lexer::TOKEN_ROWS:
                    get_next_token_();
                    break;
                case Lexer::TOKEN_MSEC:
                case Lexer::TOKEN_SEC:
                case Lexer::TOKEN_MIN:
                case Lexer::TOKEN_HOUR:
                case Lexer::TOKEN_DAY:
                    get_next_token_();
                    break;
                default:
                    break;
                }
            }

            ASSERT_TOKEN(TOKEN_RBRACKET);
            get_next_token_(); // Trash RBRACKET

            Window window(width, stride, type);
        }

    private:

        // <CONDITIONS> := <CONDITION> ((AND | OR) <CONDITIONS>)?
        Condition::ptr_t parse_conditions_() {
            Condition::ptr_t condition = parse_condition_();

            if (Lexer::is_token_conjunctive(current_token_)) {
                ConditionConjunctive::Type conjunctive_type;
                switch (current_token_) {
                case Lexer::TOKEN_AND:
                    conjunctive_type = ConditionConjunctive::AND;
                    break;
                case Lexer::TOKEN_OR:
                    conjunctive_type = ConditionConjunctive::OR;
                    break;
                default:
                    report_error_("Not handled ConditionConjunctive");
                }
                get_next_token_();
                Condition::ptr_t right_conditions = parse_conditions_();

                condition.reset(
                    new ConditionConjunctive(
                        condition,
                        right_conditions,
                        conjunctive_type
                    )
                );
            }

            return condition;
        }

        // <CONDITION> := NOT? (LPAREN <CONDITIONS> RPAREN | <COMPARISON>)
        Condition::ptr_t parse_condition_() {
            bool negated = false;
            if (current_token_ == Lexer::TOKEN_NOT) {
                negated = true;
                get_next_token_(); // Trash NOT
            }

            Condition::ptr_t condition;
            switch (current_token_) {
            case Lexer::TOKEN_LPAREN:
                get_next_token_(); // Trash LPAREN
                condition = parse_conditions_();
                ASSERT_TOKEN(TOKEN_RPAREN);
                get_next_token_(); // Trash RPAREN
                break;
            default:
                condition = parse_comparison_();
                break;
            }

            if (negated)
                condition->negate();

            return condition;
        }

        // <COMPARISON> := <VALUE> <COMPARATOR> <ATTRIBUTE> |
        //                 <ATTRIBUTE> <COMPARATOR> <VALUE> |
        //                 <ATTRIBUTE> <COMPARATOR> <ATTRIBUTE>
        Condition::ptr_t parse_comparison_() {
            if (Lexer::is_token_value(current_token_)) {
                // <VALUE> <COMPARATOR> <ATTRIBUTE>
                Object value = parse_value_();
                Comparator::Type comparator = parse_comparator_();
                AttributeIdentifier attribute_identifier = parse_attribute_();

                return Condition::ptr_t(
                    new ConditionConstantComparator(
                        attribute_identifier.name, // TODO: make Comparator to use AttributeIdentifier
                        comparator,
                        value
                    )
                );
            } else {
                AttributeIdentifier attribute_identifier = parse_attribute_();
                Comparator::Type comparator = parse_comparator_();

                if (Lexer::is_token_value(current_token_)) {
                    // <ATTRIBUTE> <COMPARATOR> <VALUE>
                    Object value = parse_value_();

                    return Condition::ptr_t(
                        new ConditionConstantComparator(
                            attribute_identifier.name,
                            comparator,
                            value
                        )
                    );
                } else {
                    // <ATTRIBUTE> <COMPARATOR> <ATTRIBUTE>
                    AttributeIdentifier attribute_identifier2 = parse_attribute_();

                    return Condition::ptr_t(
                        new ConditionAttributeComparator(
                            attribute_identifier.name,
                            comparator,
                            attribute_identifier2.name
                        )
                    );
                }
            }
        }

        // <VALUE> := STRING | INTEGER | FLOAT | <ATTRIBUTE>
        Object parse_value_() {
            if (!Lexer::is_token_value(current_token_))
                report_error_("Expected <VALUE>");

            Lexer::Token value_token = current_token_;
            std::string value_string = get_current_string_();
            get_next_token_();  // Trash <VALUE>

            std::stringstream ss;

            switch (value_token) {
            case Lexer::TOKEN_STRING:
                return Object(value_string);
            case Lexer::TOKEN_INTEGER:
                int value_int;
                ss << value_string;
                ss >> value_int;
                return Object(value_int);
            case Lexer::TOKEN_FLOAT:
                double value_float;
                ss << value_string;
                ss >> value_float;
                return Object(value_float);
            default:
                report_error_("Unknown Error");
            }
        }

        // <COMPARATOR> := EQUAL | NOT_EQUAL | LESS_THAN | LESS_THAN_EQUAL | GREATER_THAN | GREATER_THAN_EQUAL
        Comparator::Type parse_comparator_() {
            Comparator::Type comparator;

            switch (current_token_) {
            case Lexer::TOKEN_EQUAL:
                comparator = Comparator::EQUAL;
                break;
            case Lexer::TOKEN_NOT_EQUAL:
                comparator = Comparator::NOT_EQUAL;
                break;
            case Lexer::TOKEN_LESS_THAN:
                comparator = Comparator::LESS_THAN;
                break;
            case Lexer::TOKEN_LESS_THAN_EQUAL:
                comparator = Comparator::LESS_THAN_EQUAL;
                break;
            case Lexer::TOKEN_GREATER_THAN:
                comparator = Comparator::GREATER_THAN;
                break;
            case Lexer::TOKEN_GREATER_THAN_EQUAL:
                comparator = Comparator::GREATER_THAN_EQUAL;
                break;
            default:
                report_error_("Expected <COMPARATOR>");
            }

            get_next_token_(); // Trash <COMPARATOR>

            return comparator;
        }

        // ------------------------------------------------------------ //
        // DDL
        // ------------------------------------------------------------ //

        // <CREATE> := CREATE STREAM NAME (COMMA <ATTRIBUTE_DEFS>)?
        CreateNode::ptr_t parse_create_() {
            ASSERT_TOKEN(TOKEN_CREATE);
            get_next_token_();  // Trash CREATE

            // TODO: support other create types (TABLE, VIEW, TRIGGER, INDEX, ...)
            if (current_token_ != Lexer::TOKEN_STREAM && current_token_ != Lexer::TOKEN_TABLE)
                report_error_("Error: expected STREAM or TABLE"); 
            get_next_token_();  // Trash STREAM or TABLE

            ASSERT_TOKEN(TOKEN_NAME);
            std::string stream_name = get_current_string_();
            get_next_token_();  // Trash NAME

            ASSERT_TOKEN(TOKEN_LPAREN);
            get_next_token_();  // Trash LPAREN

            std::list<Attribute> attributes = parse_attribute_defs_();

            ASSERT_TOKEN(TOKEN_RPAREN);
            get_next_token_();  // Trash RPAREN

            return CreateNode::ptr_t(new CreateNode(stream_name, attributes));
        }

        // <ATTRIBUTE_DEFS> := <ATTRIBUTE_DEF> (COMMA <ATTRIBUTE_DEFS>)?
        std::list<Attribute> parse_attribute_defs_() {
            std::list<Attribute> attributes;
            attributes.push_back(parse_attribute_def_());

            if (current_token_ == Lexer::TOKEN_COMMA) {
                get_next_token_(); // Trash COMMA
                std::list<Attribute> rest_attributes = parse_attribute_defs_();
                attributes.insert(attributes.end(), rest_attributes.begin(), rest_attributes.end());
            }

            return attributes;
        }

        // <ATTRIBUTE_DEF> := <NAME> <NAME>
        Attribute parse_attribute_def_() {
            ASSERT_TOKEN(TOKEN_NAME);
            std::string attribute_name = get_current_string_();
            get_next_token_();

            ASSERT_TOKEN(TOKEN_NAME);
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
