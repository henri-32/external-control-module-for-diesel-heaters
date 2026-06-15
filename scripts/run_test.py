import subprocess
from pathlib import Path
import xml.etree.ElementTree as ET
import argparse
import sys

p = Path(__file__).resolve()
project_root = p.parent.parent
log_file = project_root / ".logs" / "test.log"
xml_file = project_root / ".logs" / "test_report.xml"
ql_file = project_root / ".logs" / "test_formatted_ql.txt"
nvim_server = Path("/tmp/nvim-main.sock")


parser = argparse.ArgumentParser()
parser.add_argument(
    "binary",
    choices=[
        "unit_tests",
        "integration_test",
        "unit_tests_debug",
        "integrationtest_debug",
    ],
    help="define testbinary to be executed",
)


def main(binary):
    if binary == "unit_tests":
        binary = project_root / "build_test" / "unit_tests"
    elif binary == "unit_tests_debug": 
      binary = project_root / "build_test_debug" / "unit_tests" 

    elif binary == "integration_test":
      binary = project_root / "build_integrationtest" / "integration_test"

    elif binary == "integrationtest_debug": 
      binary = project_root / "build_integrationtest_debug" / "integration_test"
        

    with open(log_file, "w") as f:
        process = subprocess.Popen(
            [str(binary), "--gtest_output=xml:{}".format(xml_file)],
            stdin=subprocess.DEVNULL,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )
        for line in process.stdout:
            f.write(line)

        return_code = process.wait()

    # ============= XML-File Check =====================
    if not xml_file.exists():
        raise RuntimeError("No xml_file from gtest")

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
            quickfix_lines.append("{}:{}: {}".format(file, line, msg))

    with open(ql_file, "w") as f:
        for line in quickfix_lines:
            f.write(line + "\n")

    # ============== Return Code der Tests ======================
    # Wenn nvim als server läuft werden die Tests direkt in die cf list geladn
    # und qickfix geöffnet
    if return_code != 0:
        if nvim_server.exists():
            subprocess.Popen(
                [
                    "nvim",
                    "--server",
                    str(nvim_server),
                    "--remote-send",
                    r"<C-\><C-N>:cgetfile /{}<CR>:copen<CR>".format(ql_file),
                ],
                stdin=subprocess.DEVNULL,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                start_new_session=True,
                close_fds=True,
            )

        print(
            "\nGtest Run of {}".format(binary.name),
                "failed with code {}".format(return_code), 
                "\nSee .logs/{} for complete gtest log".format(log_file.name)
            )
        
#parsed den gtest output, sodass jeder failed test nur einmal ausgegeben wird
#indem bei der Zusammenfassung die Schleife beendet wird. 
        print("\nthe failed tests are:\n")
        with open (log_file, "r") as f: 
            for line in f:
                if "FAILED" in line: 
                    if "listed below" in line: 
                        break
                    else:
                        print(line)
        
        sys.exit(1)

    else:
        print("All {} green".format(binary.name))
        sys.exit(0)


args = parser.parse_args()

if __name__ == "__main__":
    main(args.binary)
