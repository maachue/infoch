local function text_addln(x)
  print(x .. "\n") -- now print is text_add!
end

text_addln("You are running on Lua version: " .. _VERSION)

local info_str = "麻雀 (マーチュエ)"
local BOLD = "\x1b[1m"
local OFF = "\x1b[0m"

image_set("~/.face", 0, 10, 5.576, 5)

local test = false

local conf_file, err = io.open("./conf_for_lua", "r")
if conf_file then
  local str = ""

  for line in conf_file:lines() do
    str = str .. line
  end

  test = str == "true"
end

-- some text to test
text_addln("Love u <3")
text_addln("Love u too <3")

-- color?
text_addln("\x1b[31;1mA FAKE ERROR!\n\x1b[34;1mHahaha!\x1b[0m")

if os.date("*t").hour > 22 then
  text_addln("Holy shit! It is too late! Go to sleep now!")
  return
end


-- test some 5.3 standard library
text_addln("Simple string: " .. info_str)

text_addln("Simple str has length: " .. utf8.len(info_str))
for pos, code_point in utf8.codes(info_str) do
  text_addln(string.format("%02d> U+%04X", pos, code_point))
end

local STR = "💀🥀🙏🤫🤫👨‍👩‍👧‍👦"

text_addln(STR)
text_addln(utf8.len(STR) .. "<->" .. string.len(STR))

local s = STR:find("\u{200D}")
if s then
  text_addln("AA: " .. STR:sub(s))
end

STR = "👨‍👩‍👧‍👦"
s = string.find(STR, "\u{200D}")
text_addln("IS: " .. s)
if s then
  text_addln("AAA: " .. STR:sub(s))
end
