#!/usr/bin/python

#
# mvg - Model-View-Generator for Qt 4.x
#
# (c) 2010 by H.Schurig, 61206 Woellstadt, Germany
#
# This script takes a *.yaml file and creates a *.h and *.cpp file that contain all the
# boilerplate for a Qt Model/View table which stores it's data in QList<struct>.
#
# Currently possible:
# * Displaying
# * in-place edit
# TODO:
# * adding records
# * deleting records
# * editing records via QDialog
#


import sys, yaml, os

col_count = 0
col_prefix = ""
c_col = []

h_include = []
h_struct = []
h_container = []
c_container = []
h_model = []
c_model = []
h_view = []
c_view = []


def get(dict, var, default=None):
    if dict.has_key(var):
        return dict[var]
    else:
        return default


def appendCode(dest, indent, text):
    dest.append("%s// START custom code" % indent)
    for s in text.rstrip().split("\n"):
        dest.append("%s%s" % (indent, s))
    dest.append("%s// END custom code" % indent)


def generateStruct(name, cont, fields):
    global h_struct
    h_struct.append("class %s {" % name)
    h_struct.append("public:")
    for field in fields:
        try:
            h_struct.append("\t%s %s;" % (field["type"], field["name"]))
        except KeyError:
            pass
    h_struct.append("};\n\n")


def generateContainer(name, cont):
    if not cont["type"] in ("QList",):
        raise "cannot handle container type %s" % cont["type"]
    global h_include
    h_include.append("#include <%s>" % cont["type"] )
    global h_container
    h_container.append("extern %s<%s> %s;\n\n" % ( cont["type"], name, cont["name"]) )
    global c_container
    c_container.append("%s<%s> %s;\n\n" % ( cont["type"], name, cont["name"]) )


def generateColumnConsts(model, fields):
    global c_col
    global col_prefix
    global col_count
    col_prefix = "COL_%s" % model["name"].upper()
    col_count = 0
    for field in fields:
        if not field.has_key("head"):
            continue
        try:
            c_col.append("const int %s_%s = %d;" % (col_prefix, field["name"].upper(), col_count))
            col_count += 1
        except KeyError:
            pass
    c_col.append("const int %s_LAST = %d;" % (col_prefix, col_count-1))
    c_col.append("\n")


def generateModelContructor(model, code):
    global h_model
    h_model.append("\t%s(QObject *parent=0);" % model["name"])
    h_model.append("")
    global c_model
    c_model.append("%s::%s(QObject *parent)" % (model["name"], model["name"]) )
    c_model.append("\t: %s(parent)" % model["type"])
    # TODO: optional member initialization
    c_model.append("{")
    name = "construct" + model["name"]
    if code.has_key(name):
        appendCode(c_model, "\t", code[name])
    c_model.append("}\n")


def generateModelRowCount(model, cont):
    global h_model
    h_model.append("\tint rowCount(const QModelIndex &parent = QModelIndex()) const;")
    global c_model
    c_model.append("int %s::rowCount(const QModelIndex &parent) const" % model["name"])
    c_model.append("{")
    c_model.append("\tQ_UNUSED(parent);\n")
    c_model.append("\treturn %s.count();" % cont["name"])
    c_model.append("}\n")


def generateModelColumnCount(model):
    global h_model
    h_model.append("\tint columnCount(const QModelIndex &parent = QModelIndex() ) const;")
    global c_model
    c_model.append("int %s::columnCount(const QModelIndex &parent) const" % model["name"])
    c_model.append("{")
    c_model.append("\tQ_UNUSED(parent);\n")
    c_model.append("\treturn %s_LAST+1;" % col_prefix)
    c_model.append("}\n")


def generateModelHeaderData(model, fields):
    global h_model
    h_model.append("")
    h_model.append("\tQVariant headerData(int section, Qt::Orientation orientation,")
    h_model.append("\t                    int role = Qt::DisplayRole) const;")
    global c_model
    c_model.append("QVariant %s::headerData(int section, Qt::Orientation orientation, int role) const" % model["name"])
    c_model.append("{")
    c_model.append("\tif (role != Qt::DisplayRole)")
    c_model.append("\t\treturn QVariant();")
    c_model.append("")
    c_model.append("\tif (orientation == Qt::Vertical) {")
    c_model.append("\t\treturn tr(\"Row %1\").arg(section);")
    c_model.append("\t} else {")
    c_model.append("\t\tswitch (section) {")
    for field in fields:
        if field.has_key("head") and field.has_key("name"):
            c_model.append("\t\tcase %s_%s:" % (col_prefix, field["name"].upper() ))
            c_model.append("\t\t\treturn tr(\"%s\");" % field["head"] )
    c_model.append("\t\t}")
    c_model.append("\t}")
    c_model.append("\treturn QVariant();")
    c_model.append("}\n")


def generateModelData(model, fields, cont, code):
    global h_model
    h_model.append("\tQVariant data(const QModelIndex &index, int role) const;")
    global c_model
    c_model.append("QVariant %s::data(const QModelIndex &index, int role) const" % model["name"])
    c_model.append("{")
    c_model.append("\tif (!index.isValid())")
    c_model.append("\t\treturn QVariant();")
    c_model.append("")
    c_model.append("\tint row = index.row();")
    c_model.append("")
    c_model.append("\tif (row >= %s.count())" % cont["name"])
    c_model.append("\t\treturn QVariant();")
    c_model.append("")
    c_model.append("\tif (role == Qt::DisplayRole) {")
    c_model.append("\t\tswitch (index.column()) {")
    col_checkbox = False
    for field in fields:
        if not field.has_key("head"):
            continue
        if get(field, "table_type") == "checkbox":
            col_checkbox = True
            continue
        else:
            c_model.append("\t\tcase %s_%s:" % (col_prefix, field["name"].upper()) )
        if field.has_key("data_code"):
            c_model.append("\t\t\t{")
            appendCode(c_model, "\t\t\t", code[field["data_code"]])
            c_model.append("\t\t\t}")
        elif field.has_key("type"):
            c_model.append("\t\t\treturn %s[row].%s;" % (
                    cont["name"],
                    field["name"]))
        else:
            c_model.append("\t\t\treturn QVariant(); //TODO")
    c_model.append("\t\t}")
    if col_checkbox:
        c_model.append("\t} else")
        c_model.append("\tif (role == Qt::CheckStateRole) {")
        c_model.append("\t\tswitch (index.column()) {")
        for field in fields:
            if get(field, "table_type") != "checkbox":
                continue
            c_model.append("\t\tcase %s_%s: {" % (col_prefix, field["name"].upper()) )
            c_model.append("\t\t\tbool checked = %s[row].%s;" % (cont["name"], field["name"]))
            c_model.append("\t\t\treturn checked ? Qt::Checked : Qt::Unchecked;")
            c_model.append("\t\t\t}")
        c_model.append("\t\t}")
    c_model.append("\t}")
    c_model.append("")
    c_model.append("\treturn QVariant();")
    c_model.append("}\n")


def generateModelSort(name, model, cont, fields, code):
    global h_model
    # TODO: if sort:
    h_model.append("")
    h_model.append("\t// Sort support:")
    h_model.append("\tvirtual void sort(int column, Qt::SortOrder order=Qt::AscendingOrder);")

    global c_model

    # First the static sort functions
    for field in fields:
        if not field.has_key("head"):
            continue
        c_model.append("static bool sort%s%s(const %s &one, const %s two)" % (
                model["name"], field["name"].capitalize(),
                name, name))
        c_model.append("{")
        order = get(field, "order", "ascending")
        if order == "ascending":
            order = "Qt::AscendingOrder"
        elif order == "descending":
            order = "Qt::DescendingOrder";
        s1 = "one.%s" % field["name"]
        s2 = "two.%s" % field["name"]
        if field.has_key("sort_code"):
            s1 = "s1"
            s2 = "s2"
            appendCode(c_model, "\t", code[field["sort_code"]])
        c_model.append("\tif (sortOrder == %s)" % order)
        c_model.append("\t\treturn %s < %s;" % (s1, s2))
        c_model.append("\telse")
        c_model.append("\t\treturn %s > %s;" % (s1, s2))
        c_model.append("}\n")

    # and then the class member:
    c_model.append("void %s::sort(int column, Qt::SortOrder order)" % model["name"])
    c_model.append("{")
    c_model.append("\tsortOrder = order;")
    c_model.append("")
    c_model.append("\tswitch (column) {")
    for field in fields:
        if not field.has_key("head"):
            continue
        c_model.append("\tcase %s_%s:" % (col_prefix, field["name"].upper()) )
        c_model.append("\t\tqStableSort(%s.begin(), %s.end(), sort%s%s);" % (
                cont["name"], cont["name"],
                model["name"], field["name"].capitalize(),
                ))
        c_model.append("\t\tbreak;")
    c_model.append("\tdefault:")
    c_model.append("\t\treturn;")
    c_model.append("\t}")
    c_model.append("\temit dataChanged(index(0, 0), index(chars.count(), %s_LAST));" % col_prefix)
    c_model.append("}\n")


def generateModelEdit(model, fields, cont):
    global h_model
    # TODO: if edit:
    h_model.append("")
    h_model.append("\t// Edit support:")
    h_model.append("\tvoid store(const QString &sign, const QString &code);")
    generateModelFlags(model, fields)
    generateModelSetData(model, fields, cont)


def generateModelFlags(model, fields):
    global h_model
    h_model.append("\tQt::ItemFlags flags(const QModelIndex &index) const;");
    global c_model
    c_model.append("Qt::ItemFlags %s::flags(const QModelIndex &index) const" % model["name"])
    c_model.append("{")
    c_model.append("\tif (!index.isValid())")
    c_model.append("\t\treturn Qt::ItemIsEnabled;")
    c_model.append("")
    c_model.append("\tQt::ItemFlags flags = QAbstractItemModel::flags(index);")
    c_model.append("\tflags |= Qt::ItemIsEnabled;")
    c_model.append("")
    c_model.append("\tswitch (index.column()) {")
    for field in fields:
        if not field.has_key("head") or get(field, "readonly"):
            continue
        c_model.append("\tcase %s_%s:" % (col_prefix, field["name"].upper()) )
        if get(field, "table_type") == "checkbox":
            c_model.append("\t\tflags |= Qt::ItemIsUserCheckable;")
        c_model.append("\t\tflags |= Qt::ItemIsEditable;")
        c_model.append("\t\tbreak;")
    c_model.append("\t}")
    c_model.append("\treturn flags;")
    c_model.append("}\n")


def generateModelSetData(model, fields, cont):
    global h_model
    h_model.append("\tbool setData(const QModelIndex &index, const QVariant &value,")
    h_model.append("\t             int role = Qt::EditRole);");
    global c_model
    c_model.append("bool %s::setData(const QModelIndex &index, const QVariant &value," % model["name"])
    c_model.append("\tint role)")
    c_model.append("{")
    c_model.append("\tif (!index.isValid())")
    c_model.append("\t\treturn false;")
    c_model.append("\tint row = index.row();")
    c_model.append("")
    c_model.append("\tif (role == Qt::EditRole) {")
    c_model.append("\t\tswitch (index.column()) {")
    has_checkbox = False
    for field in fields:
        if not field.has_key("head") or get(field, "readonly"):
            continue
        if get(field, "table_type") == "checkbox":
            has_checkbox = True
            continue
        c_model.append("\t\tcase %s_%s:" % (col_prefix, field["name"].upper()) )
        conv = ""
        if field["type"] == "QString":
            conv = "toString"
        elif field["type"] == "quint32":
            conv = "toUInt"
        else:
            raise "Unhandled field type %s" % field["type"]
        c_model.append("\t\t\t%s[row].%s = value.%s();" % (
                cont["name"],
                field["name"],
                conv,
                ))
        c_model.append("\t\t\tbreak;")
    c_model.append("\t\tdefault:")
    c_model.append("\t\t\treturn false;")
    c_model.append("\t\t}")
    c_model.append("\t\temit dataChanged(index, index);")
    c_model.append("\t\treturn true;")
    c_model.append("\t}")
    if has_checkbox:
        c_model.append("\tif (role == Qt::CheckStateRole) {")
        for field in fields:
            if not field.has_key("head") or get(field, "readonly"):
                continue
            if get(field, "table_type") != "checkbox":
                continue
            c_model.append("\t\tswitch (index.column()) {")
            c_model.append("\t\tcase %s_%s:" % (col_prefix, field["name"].upper()) )
            c_model.append("\t\t\tchars[row].enabled = value.toBool();")
            c_model.append("\t\t\temit dataChanged(index, index);")
            c_model.append("\t\t\treturn true;")
        c_model.append("\t\t}")
        c_model.append("\t}")
    c_model.append("\treturn false;")
    c_model.append("}\n")


def generateModel(name, model, cont, fields, code):
    if not model["type"] in ("QAbstractTableModel",):
        raise "cannot handle model type %s" % model["type"]
    global h_include
    h_include.append("#include <%s>" % model["type"] )

    global h_model
    h_model.append("class %s : public %s" % (model["name"], model["type"]) )
    h_model.append("{")
    h_model.append("\tQ_OBJECT")
    h_model.append("public:")

    generateColumnConsts(model, fields)
    generateModelContructor(model, code)
    generateModelRowCount(model, cont)
    generateModelColumnCount(model)
    generateModelHeaderData(model, fields)
    generateModelData(model, fields, cont, code)
    generateModelSort(name, model, cont, fields, code)
    generateModelEdit(model, fields, cont)
    # TODO: members
    h_model.append("};\n\n")
    c_model.append("")


def generateView(view):
    global h_include
    h_include.append("#include <%s>" % view["type"])
    global h_view
    h_view.append("class %s : public %s" % (view["name"], view["type"]))
    h_view.append("{")
    h_view.append("	Q_OBJECT")
    h_view.append("public:")
    h_view.append("	%s(QWidget *parent=0);" % view["name"])
    # TODO: members
    h_view.append("};\n")

    global c_view
    c_view.append("%s::%s(QWidget *parent)" % (view["name"],view["name"]))
    c_view.append("\t: %s(parent)" % view["type"])
    # TODO: members
    c_view.append("{")
    c_view.append("	setAlternatingRowColors(true);")
    c_view.append("	verticalHeader()->hide();")
    c_view.append("	horizontalHeader()->setResizeMode(QHeaderView::Stretch);")
    c_view.append("	setTabKeyNavigation(false);")
    c_view.append("\tsetHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);")
    c_view.append("\tsetVerticalScrollMode(QAbstractItemView::ScrollPerPixel);")
    c_view.append("")
    # TODO: if sort...
    c_view.append("\tconnect(horizontalHeader(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)),")
    c_view.append("\t        SLOT(sortByColumn(int)) );")
    c_view.append("\tsetSortingEnabled(true);")
    c_view.append("\tsortByColumn(0, Qt::AscendingOrder);")
    c_view.append("}\n")





from optparse import OptionParser
parser = OptionParser()
parser.add_option("-o", "--output", dest="dest",
	help="write output to FILE", metavar="FILE")
parser.add_option("--impl",
                  action="store_true", dest="impl", default=False,
                  help="generate implementation (*.cpp)")
(options, args) = parser.parse_args()

if len(args) != 1 or not options.dest:
    print "Usage: %s -o <output> <filename>.yaml" % sys.argv[0]

basename = os.path.splitext(os.path.basename(args[0]))[0]
stream = open(args[0])
data = yaml.load(stream, Loader=yaml.CLoader)
#print yaml.dump(data, Dumper=yaml.CDumper)

for name in data:
    item = data[name]
    generateStruct(name, item["container"], item["fields"])
    generateContainer(name, item["container"])
    if not item.has_key("code"):
        item["code"] = {}
    if item.has_key("model"):
        generateModel(name, item["model"], item["container"], item["fields"], item["code"])
    if item.has_key("view"):
        generateView(item["view"])
h_include.append("\n")

if options.impl:
    f = open(options.dest, "w")
    f.write("#include \"%s.h\"\n" % basename)
    f.write("#include <QHeaderView>\n\n")
    f.write("\n".join(c_container))
    f.write("\n".join(c_col))
    # TODO: if sort
    f.write("static Qt::SortOrder sortOrder = Qt::AscendingOrder;\n\n")
    f.write("\n".join(c_model))
    f.write("\n".join(c_view))
else:
    f = open(options.dest, "w")
    f.write("#ifndef %s_H\n" % basename.upper())
    f.write("#define %s_H\n\n" % basename.upper())
    f.write("\n".join(h_include))
    f.write("\n".join(h_struct))
    f.write("\n".join(h_container))
    f.write("\n".join(h_model))
    f.write("\n".join(h_view))
    f.write("\n#endif\n")

#print "\n".join(h_include)
#print "\n".join(h_struct)
#print "\n".join(h_container)
#print "\n".join(h_model)
#print "\n".join(h_view)
#print "\n".join(c_container)
#print "\n".join(c_col)
#print "\n".join(c_model)
#print "\n".join(c_view)
