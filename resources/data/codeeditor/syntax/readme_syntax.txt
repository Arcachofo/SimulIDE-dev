First line must be a list of rules.
Then a rule-style and a rule RegExp or Word List for each rule.

rules: list of rules

rule-style: foregroundColor backgroundColor bold italic
rule: in a single line 

____________________________________________________________

Special case is "multiLineComment" rule.
It needs start and end RegExps, for example:

multiLineComment-style: #00a651 default false true
multiLineComment: "/\\*" "\\*/"



##############  AVAILABLE OPTIONS:  ################################

Colors:  { default | #RRGGBB }
Bold:    { true | false }
Italic:  { true | false }

RegExp: must be quotated, example: "#[a-zA-Z]+\b"

