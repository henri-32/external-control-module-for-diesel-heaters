import subprocess 
from pathlib import Path
import xml.etree.ElementTree as ET

p = Path(__file__).resolve()
project_root = p.parent.parent
binary = project_root / "build_test" / "unit_tests" 
log_file = project_root / ".logs" / "unit_tests.log"
xml_file = project_root / ".logs" / "unit_tests_report.xml"
ql_file = project_root / "neovim_utils" / "quickfix_list.txt" 

if not log_file.exists(): 
    open(log_file, "x") 

with open (log_file, "w") as f: 
    process = subprocess.Popen(
        [str(binary), "--gtest_output=xml:{}".format(xml_file)],
        stdout=subprocess.PIPE, 
        stderr=subprocess.STDOUT,
        text=True
    )
    for line in process.stdout: 
         f.write(line)
	
    return_code = process.wait()

#============= XML-File Check =====================
if not xml_file.exists(): 
    raise RuntimeError("No xml_file")

# ============ XML Parsen für Neovim Quickfix ==================== 
tree = ET.parse(xml_file)
root_xml = tree.getroot()

quickfix_lines = [] 

for testcase in root_xml.iter("testcase"): 
    file = testcase.attrib.get("file")
    line = testcase.attrib.get("line")

    if not file or not line: 
        continue 

    for failure in testcase.findall("failure"): 
        msg = failure.attrib.get("message", "")
        msg = msg.replace("\n", " ")
        quickfix_lines.append("{}:{}: {}".format(file, line, msg)
)

if not ql_file.exists(): 
    open(ql_file, "x")

with open(ql_file, "w") as f:
    for line in quickfix_lines: 
        f.write(line + "\n")

# ============== Return Code der Tests ======================
if return_code != 0: 
    raise RuntimeError("Gtest Run failed with code {}".format(return_code))
else: 
    print("All tests green")


