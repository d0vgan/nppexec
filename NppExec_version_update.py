import os

VERSION_TO_SET = "0.7.9.9"
DATE_TO_SET = "January 2022"


def run():
    update_version_and_date("NppExec/doc/NppExec/NppExec.txt", True)
    update_version_and_date("NppExec/doc/NppExec/NppExec_TechInfo.txt", False)
    update_version_and_date("NppExec/src/NppExec.cpp", False)
    update_h_file("NppExec/src/NppExec.h")
    update_rc_file("NppExec/src/NppExec.rc")


def update_version_and_date(filename: str, update_verdate: bool):
    path_to_file = os.path.normpath(os.path.join(get_py_file_dir(), filename))
    with open(path_to_file) as f:
        lines = f.readlines()
    is_ver_replaced = False
    is_date_replaced = False
    is_ver_date_replaced = False if update_verdate else True
    line_idx = 0
    for line in lines:
        if not is_ver_replaced and line.startswith(" * NppExec plugin ver. "):
            pos_1 = line.find("ver. ") + 5
            pos_2 = line.find(" for ")
            lines[line_idx] = line[0:pos_1] + VERSION_TO_SET + line[pos_2:]
            is_ver_replaced = True
        elif not is_date_replaced and line.startswith(" * by DV <dvv81 @ ukr.net>, December 2006 - "):
            pos_1 = line.find(" - ") + 3
            lines[line_idx] = line[0:pos_1] + DATE_TO_SET + get_line_end(line)
            is_date_replaced = True
        elif not is_ver_date_replaced and line.startswith(" v{0} ".format(VERSION_TO_SET)):
            line_updated = " v{0} - {1}".format(VERSION_TO_SET, DATE_TO_SET)
            line_end = get_line_end(line)
            lines[line_idx] = line_updated + line_end
            lines[line_idx + 1] = " " + "-"*(len(line_updated) - 1) + line_end
            is_ver_date_replaced = True
        if is_ver_replaced and is_date_replaced and is_ver_date_replaced:
            break
        line_idx += 1
    if not is_ver_replaced:
        warning(filename, "version was not replaced")
    if not is_date_replaced:
        warning(filename, "date was not replaced")
    if not is_ver_date_replaced:
        warning(filename, "version_date was not replaced")
    with open(path_to_file, "w") as f:
        f.writelines(lines)


def update_h_file(filename: str):
    path_to_file = os.path.normpath(os.path.join(get_py_file_dir(), filename))
    with open(path_to_file) as f:
        lines = f.readlines()
    is_ver_date_replaced = False
    is_ver_dword_replaced = False
    is_ver_str_replaced = False
    line_idx = 0
    for line in lines:
        if not is_ver_date_replaced and line.startswith(" v{0} ".format(VERSION_TO_SET)):
            line_updated = " v{0} - {1}".format(VERSION_TO_SET, DATE_TO_SET)
            line_end = get_line_end(line)
            lines[line_idx] = line_updated + line_end
            lines[line_idx + 1] = " " + "-"*(len(line_updated) - 1) + line_end
            is_ver_date_replaced = True
        elif not is_ver_dword_replaced and line.startswith("#define NPPEXEC_VER_DWORD "):
            int_ver = get_int_ver(VERSION_TO_SET)
            pos_1 = line.find("DWORD ") + 6
            lines[line_idx] = line[0:pos_1] + "0x%X%XF%X" % (int_ver[0], int_ver[1], int_ver[2]) + get_line_end(line)
            is_ver_dword_replaced = True
        elif not is_ver_str_replaced and line.startswith("#define NPPEXEC_VER_STR "):
            pos_1 = line.find('_T("') + 4
            pos_2 = line.find('")')
            lines[line_idx] = line[0:pos_1] + VERSION_TO_SET + line[pos_2:]
            is_ver_str_replaced = True
        if is_ver_date_replaced and is_ver_dword_replaced and is_ver_str_replaced:
            break
        line_idx += 1
    if not is_ver_date_replaced:
        warning(filename, "version_date was not replaced")
    if not is_ver_dword_replaced:
        warning(filename, "version_dword was not replaced")
    if not is_ver_str_replaced:
        warning(filename, "version_str was not replaced")
    with open(path_to_file, "w") as f:
        f.writelines(lines)


def update_rc_file(filename: str):
    path_to_file = os.path.normpath(os.path.join(get_py_file_dir(), filename))
    with open(path_to_file) as f:
        lines = f.readlines()
    is_ver_unicode_replaced = False
    is_ver_ansi_replaced = False
    is_date_replaced = False
    is_filever_num_replaced = False
    is_prodver_num_replaced = False
    is_filever_str_replaced = False
    is_prodver_str_replaced = False
    is_copyright_replaced = False
    int_ver = get_int_ver(VERSION_TO_SET)
    ver_commas = "{0}, {1}, {2}, {3}".format(int_ver[0], int_ver[1], int_ver[2], int_ver[3])
    short_date = get_short_date(DATE_TO_SET)
    line_idx = 0
    for line in lines:
        if not is_ver_unicode_replaced and line.startswith('    CTEXT           "NppExec ver. ') and line.find(" Unicode ") != -1:
            pos_1 = line.find("ver. ") + 5
            pos_2 = line.find(" Unicode ")
            lines[line_idx] = line[0:pos_1] + VERSION_TO_SET + line[pos_2:]
            is_ver_unicode_replaced = True
        elif not is_ver_ansi_replaced and line.startswith('    CTEXT           "NppExec ver. ') and line.find(" ANSI ") != -1:
            pos_1 = line.find("ver. ") + 5
            pos_2 = line.find(" ANSI ")
            lines[line_idx] = line[0:pos_1] + VERSION_TO_SET + line[pos_2:]
            is_ver_ansi_replaced = True
        elif not is_filever_num_replaced and line.startswith(" FILEVERSION "):
            pos_1 = line.find("N ") + 2
            lines[line_idx] = line[0:pos_1] + ver_commas + get_line_end(line)
            is_filever_num_replaced = True
        elif not is_prodver_num_replaced and line.startswith(" PRODUCTVERSION "):
            pos_1 = line.find("N ") + 2
            lines[line_idx] = line[0:pos_1] + ver_commas + get_line_end(line)
            is_prodver_num_replaced = True
        elif not is_filever_str_replaced and line.startswith('            VALUE "FileVersion", '):
            pos_1 = line.find(', "') + 3
            lines[line_idx] = line[0:pos_1] + ver_commas + '"' + get_line_end(line)
            is_filever_str_replaced = True
        elif not is_prodver_str_replaced and line.startswith('            VALUE "ProductVersion", '):
            pos_1 = line.find(', "') + 3
            lines[line_idx] = line[0:pos_1] + ver_commas + '"' + get_line_end(line)
            is_prodver_str_replaced = True
        elif not is_date_replaced and line.startswith('                    CTEXT           "(C)  Dec 2006 - '):
            pos_1 = line.find(" - ") + 3
            pos_2 = line.find(",  Vit")
            lines[line_idx] = line[0:pos_1] + short_date + line[pos_2:]
            is_date_replaced = True
        elif not is_copyright_replaced and line.startswith('            VALUE "LegalCopyright", '):
            pos_1 = line.find(" - ") + 3
            pos_2 = line.find(", Vit")
            lines[line_idx] = line[0:pos_1] + short_date + line[pos_2:]
            is_copyright_replaced = True
        if is_ver_unicode_replaced and is_ver_ansi_replaced and \
           is_filever_num_replaced and is_prodver_num_replaced and \
           is_filever_str_replaced and is_prodver_str_replaced and \
           is_date_replaced and is_copyright_replaced:
            break
        line_idx += 1
    if not is_ver_unicode_replaced:
        warning(filename, "version_unicode was not replaced")
    if not is_ver_ansi_replaced:
        warning(filename, "version_ansi was not replaced")
    if not is_date_replaced:
        warning(filename, "date was not replaced")
    if not is_filever_num_replaced:
        warning(filename, "filever_num was not replaced")
    if not is_prodver_num_replaced:
        warning(filename, "prodver_num was not replaced")
    if not is_filever_str_replaced:
        warning(filename, "filever_str was not replaced")
    if not is_prodver_str_replaced:
        warning(filename, "prodver_str was not replaced")
    if not is_copyright_replaced:
        warning(filename, "copyright was not replaced")
    with open(path_to_file, "w") as f:
        f.writelines(lines)


def get_py_file_dir():
    py_file_path = os.path.realpath(__file__)
    return os.path.dirname(py_file_path)


def get_int_ver(ver: str):
    int_ver = []
    str_ver = ver.split(".")
    for s in str_ver:
        int_ver.append(int(s))
    while len(int_ver) < 4:
        int_ver.append(0)
    return int_ver


def get_short_date(date: str):
    a_date = date.split(" ")
    return "{0} {1}".format(a_date[0][0:3], a_date[1])


def get_line_end(line: str):
    i = -1
    while line[i].isspace():
        i -= 1
    if i == -1:
        return ""  # no trailing whitespaces
    return line[i+1:]  # includes the trailing '\n' and '\r', if any


def warning(filename: str, message: str):
    print("Warning: {0}: {1}".format(filename, message))


run()
