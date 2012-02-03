// -*- c++ -*-

#ifndef CURRENTIA_QUERY_PARSER_H_
#define CURRENTIA_QUERY_PARSER_H_

#include "currentia/query/lexer.h"
#include "currentia/trait/non-copyable.h"

#include "currentia/query/ast.h"

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
        std::list<AttributeIdentifier> parse_attributes_() {
            std::list<AttributeIdentifier> attribute_identifiers;

            AttributeIdentifier attribute_identifier = parse_attribute_();
            attribute_identifiers.push_back(attribute_identifier);

            if (current_token_ == Lexer::COMMA) {
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

            return AttributeIdentifier(attribute_name, stream_name);
        }

        // <FROM_ELEMENTS> := <FROM_ELEMENT> (COMMA <ATTRIBUTES>)?
        void parse_from_elements_() {
            std::list<SelectionNode::ptr_t> from_elements;
            from_elements.push_back(parse_from_element_());

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

        // TODO: to testing
    public:
        Condition::ptr_t parse_conditions() {
            get_next_token_();
            return parse_conditions_();
        }
    private:

        // <CONDITIONS> := <CONDITION> ((AND | OR) <CONDITIONS>)?
        Condition::ptr_t parse_conditions_() {
            Condition::ptr_t condition = parse_condition_();

            if (Lexer::is_token_conjunctive(current_token_)) {
                ConditionConjunctive::Type conjunctive_type;
                switch (current_token_) {
                case Lexer::AND:
                    conjunctive_type = ConditionConjunctive::AND;
                    break;
                case Lexer::OR:
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
            if (current_token_ == Lexer::NOT) {
                negated = true;
                get_next_token_(); // Trash NOT
            }

            Condition::ptr_t condition;
            switch (current_token_) {
            case Lexer::LPAREN:
                get_next_token_(); // Trash LPAREN
                condition = parse_conditions_();
                ASSERT_TOKEN(RPAREN);
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
            case Lexer::STRING:
                return Object(value_string);
            case Lexer::INTEGER:
                int value_int;
                ss << value_string;
                ss >> value_int;
                return Object(value_int);
            case Lexer::FLOAT:
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
            case Lexer::EQUAL:
                comparator = Comparator::EQUAL;
                break;
            case Lexer::NOT_EQUAL:
                comparator = Comparator::NOT_EQUAL;
                break;
            case Lexer::LESS_THAN:
                comparator = Comparator::LESS_THAN;
                break;
            case Lexer::LESS_THAN_EQUAL:
                comparator = Comparator::LESS_THAN_EQUAL;
                break;
            case Lexer::GREATER_THAN:
                comparator = Comparator::GREATER_THAN;
                break;
            case Lexer::GREATER_THAN_EQUAL:
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
