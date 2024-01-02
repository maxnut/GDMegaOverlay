import sys
from pathlib import Path

def replace_mentions_of_macro(text, macro, command):
	# these must preserve line order, per input filter rules
	group_start = f"/** @{{ */ /** \\{command} */"
	group_end = f"/** @}} */"

	while macro in text:
		first = text.find(macro)
		end = text.find(")", first)

		# resolve nested parenthesis, the bad way
		p_group_start = text.rfind("(", first + len(macro) + 1, end)
		while p_group_start != -1:
			end = text.find(")", end + 1)
			p_group_start = text.rfind("(", p_group_start + 1, end)

		# cut string from start of macro to end of macro, and then to end of argument to the closing parenthesis, to strip the closing parenthesis
		text = text[0:first:] + group_start + text[first + len(macro):end:] + group_end + text[end+1::]

		if end == -1:
			raise IndexError("closing tag not found for macro definition")

	return text

def parse_file(text):

	text = replace_mentions_of_macro(text, "RT_ADD(", "robtop")
	text = replace_mentions_of_macro(text, "HJ_ADD(", "hjfod")

	return text

def _main(path):
	input_path = Path(path)

	# for some reason the cocos files just randomly mix encodings sometimes. don't ask why
	text = input_path.read_text(errors="ignore")

	# ignore ccMacros file
	if input_path.stem == "ccMacros":
		return text

	output = parse_file(text)

	return output

if __name__ == "__main__":
	input_path = sys.argv[1]
	try:
		output = _main(input_path)
		print(output)
	except Exception as e:
		print(f"Error found when parsing file {input_path}\n{e}", file=sys.stderr)
		sys.exit(1)
