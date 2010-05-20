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
has_sort = False

h_include = []
h_classes = []
c_include = []
h_struct = []
h_container = []
c_container = []
h_model = []
c_model = []
h_view = []
c_view = []
h_dialog = []
c_dialog = []

def get(dict, var, default=None):
    """Returns @dict[@var]" if it exists, otherwise returns the default."""

    if dict.has_key(var):
        return dict[var]
    else:
        return default


def appendCode(dest, indent, text):
    """Adds a pice of source code from @text into @dest. The code
    get's formatted somewhat, e.g. @indent is added before each
    line."""

    dest.append("%s// START custom code" % indent)
    for s in text.rstrip().split("\n"):
        dest.append("%s%s" % (indent, s))
    dest.append("%s// END custom code" % indent)


def addInclude(arr, header):
    """Adds an '#include <>' line to @arr, but only if it doesn't yet
    exist there."""

    header = "#include <%s>" % header
    if not header in arr:
        arr.append(header)

def addClassRef(clazz):
    """Adds an 'class @clazz;' line to h_classes, but only if it
    doesn't yet exist there."""

    global h_classes
    header = "class %s;" % clazz
    if not header in h_classes:
        h_classes.append(header)

def generateStruct(data):
    "Creates the struct that contains the data fields"

    global h_struct
    h_struct.append("class %s {" % data["name"])
    h_struct.append("public:")
    for field in data["fields"]:
        try:
            h_struct.append("\t%s %s;" % (field["type"], field["name"]))
        except KeyError:
            pass
    h_struct.append("};\n\n")


def generateContainer(data):
    "Creates the container (usually QLIst) that contains the structs."

    cont = data["container"]
    global h_include
    addInclude(h_include, cont["type"])
    global h_container
    h_container.append("extern %s<%s> %s;\n\n" % (cont["type"], data["name"], cont["name"]) )
    global c_container
    c_container.append("%s<%s> %s;\n\n" % ( cont["type"], data["name"], cont["name"]) )


def generateColumnConsts(data):
    """Generates "const int COL_xxx = n;" constants for easier
    referencing to the columns."""

    model = data["model"]
    global c_col
    global col_prefix
    global col_count
    col_prefix = "COL_%s" % model["name"].upper()
    col_count = 0
    for field in data["fields"]:
        if not field.has_key("head"):
            continue
        try:
            c_col.append("const int %s_%s = %d;" % (col_prefix, field["name"].upper(), col_count))
            col_count += 1
        except KeyError:
            pass
    c_col.append("const int %s_LAST = %d;" % (col_prefix, col_count-1))
    c_col.append("\n")


def generateModelContructor(data):
    "Creates the (boring) constructor of the model."

    model = data["model"]
    code = data["code"]
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


def generateModelRowCount(data):
    """Creates an interface function that simply returns number of
    elements in the container."""

    model = data["model"]
    cont = data["container"]
    global h_model
    h_model.append("\tint rowCount(const QModelIndex &parent = QModelIndex()) const;")
    global c_model
    c_model.append("int %s::rowCount(const QModelIndex &parent) const" % model["name"])
    c_model.append("{")
    c_model.append("\tQ_UNUSED(parent);\n")
    c_model.append("\treturn %s.count();" % cont["name"])
    c_model.append("}\n")


def generateModelColumnCount(data):
    """Creates a (boring) function that simply return the number of
    columns. We can use our COL_xxx constant from above."""

    model = data["model"]
    global h_model
    h_model.append("\tint columnCount(const QModelIndex &parent = QModelIndex() ) const;")
    global c_model
    c_model.append("int %s::columnCount(const QModelIndex &parent) const" % model["name"])
    c_model.append("{")
    c_model.append("\tQ_UNUSED(parent);\n")
    c_model.append("\treturn %s_LAST+1;" % col_prefix)
    c_model.append("}\n")


def generateModelHeaderData(data):
    """Create the ::headerData() method, which returns the row
    headers. Any field that has both a 'head' and 'name' entry is
    eligible for display."""

    model = data["model"]
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
    for field in data["fields"]:
        if field.has_key("head") and field.has_key("name"):
            c_model.append("\t\tcase %s_%s:" % (col_prefix, field["name"].upper() ))
            c_model.append("\t\t\treturn tr(\"%s\");" % field["head"] )
    c_model.append("\t\t}")
    c_model.append("\t}")
    c_model.append("\treturn QVariant();")
    c_model.append("}\n")


def generateModelData(data):
    """Create the ::data() method, which returns the current data item. Can produce calculated
    results (in the presence of a 'data_code' field entry) or checkboxes."""

    model = data["model"]
    cont = data["container"]
    code = data["code"]
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
    col_align = False
    for field in data["fields"]:
        if not field.has_key("head"):
            continue
        if field.has_key("halign") or field.has_key("valign"):
            col_align = True
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
        for field in data["fields"]:
            if get(field, "table_type") != "checkbox":
                continue
            c_model.append("\t\tcase %s_%s: {" % (col_prefix, field["name"].upper()) )
            c_model.append("\t\t\tbool checked = %s[row].%s;" % (cont["name"], field["name"]))
            c_model.append("\t\t\treturn checked ? Qt::Checked : Qt::Unchecked;")
            c_model.append("\t\t\t}")
        c_model.append("\t\t}")
    if col_align:
        c_model.append("\t} else")
        c_model.append("\tif (role == Qt::TextAlignmentRole) {")
        c_model.append("\t\tswitch (index.column()) {")
        for field in data["fields"]:
            halign = get(field, "halign", "left").capitalize()
            valign = get(field, "valign", "vcenter").capitalize()
            if halign == "Left" and valign == "Vcenter":
                continue
            if valign == "Vcenter": valign = "VCenter"
            c_model.append("\t\tcase %s_%s:" % (col_prefix, field["name"].upper()) )
            c_model.append("\t\t\treturn (int)(Qt::Align%s | Qt::Align%s);" % (halign.capitalize(), valign))
        c_model.append("\t\t}")
    c_model.append("\t}")
    c_model.append("")
    c_model.append("\treturn QVariant();")
    c_model.append("}\n")


def generateModelSort(data):
    """Sorting colums is most tedious when written by hand. This is
    where this generater excells! Can use code for custom sorting
    ('sort_code') or a different default sort order ('sort_order)."""

    model = data["model"]
    cont = data["container"]
    code = data["code"]
    global h_model
    h_model.append("")
    h_model.append("\t// Sort support:")
    h_model.append("\tvirtual void sort(int column, Qt::SortOrder order=Qt::AscendingOrder);")

    global c_model

    # First the static sort functions
    for field in data["fields"]:
        if not field.has_key("head"):
            continue
        c_model.append("static bool sort%s%s(const %s &one, const %s two)" % (
                model["name"], field["name"].capitalize(),
                name, name))
        c_model.append("{")
        order = get(field, "sort_order", "ascending")
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
    for field in data["fields"]:
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


def generateModelEdit(data):
    """This calls generators for all functions that are needed for in-place edit."""

    global h_model
    h_model.append("")
    h_model.append("\t// In-Table edit support:")
    h_model.append("\tvoid store(const QString &sign, const QString &code);")
    generateModelFlags(data)
    generateModelSetData(data)


def generateModelFlags(data):
    """This return the Qt::ItemFlags for a field. The .yaml can define
    a field as 'readonly', set the 'table_type' or 'checkbox'."""

    model = data["model"]
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
    for field in data["fields"]:
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


def generateModelSetData(data):
    """This genererates the ::setData() method, which takes the
    in-table edited user input and updates the structure in the
    container."""

    model = data["model"]
    cont = data["container"]
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
    for field in data["fields"]:
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
        for field in data["fields"]:
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


def generateModelInsertRows(data):
    model = data["model"]
    cont = data["container"]
    global h_model
    h_model.append("\tvirtual bool insertRows(int row, int count, const QModelIndex &parent=QModelIndex());")
    global c_model
    c_model.append("bool %s::insertRows(int row, int count, const QModelIndex &parent)" % model["name"]);
    c_model.append("{")
    c_model.append("\tbeginInsertRows(parent, row, row+1);");
    c_model.append("\t%s m;" % data["name"])
    c_model.append("\tint n = count;")
    c_model.append("\twhile (n--)")
    c_model.append("\t\t%s.insert(row, m);" % cont["name"])
    c_model.append("\tendInsertRows();");
    c_model.append("\treturn true;\n")
    c_model.append("}\n")


def generateModelRemoveRows(data):
    model = data["model"]
    cont = data["container"]
    global h_model
    h_model.append("\tvirtual bool removeRows(int row, int count, const QModelIndex &parent=QModelIndex());")
    global c_model
    c_model.append("bool %s::removeRows(int row, int count, const QModelIndex &parent)" % model["name"]);
    c_model.append("{")
    c_model.append("\tbeginRemoveRows(parent, row, row+count-1);");
    c_model.append("\tint n = count;")
    c_model.append("\twhile (n--)")
    c_model.append("\t\t%s.removeAt(row);" % cont["name"])
    c_model.append("\tendRemoveRows();");
    c_model.append("\treturn true;\n")
    c_model.append("}\n")


def generateModel(data):
    """This is the high-level function which generates the whole model."""

    model = data["model"]
    view = get(data, "view", {})
    if not model["type"] in ("QAbstractTableModel",):
        raise "cannot handle model type %s" % model["type"]
    global h_include
    addInclude(h_include, model["type"])

    global h_model
    h_model.append("class %s : public %s" % (model["name"], model["type"]) )
    h_model.append("{")
    h_model.append("\tQ_OBJECT")
    h_model.append("public:")

    generateColumnConsts(data)
    generateModelContructor(data)
    generateModelRowCount(data)
    generateModelColumnCount(data)
    generateModelHeaderData(data)
    generateModelData(data)
    if get(model, "sort", True):
        global has_sort
        has_sort = True
        generateModelSort(data)
    if get(model, "edit_table", True):
        generateModelEdit(data)
    if get(view, "insert"):
        generateModelInsertRows(data)
    if get(view, "delete"):
        generateModelRemoveRows(data)
    # TODO: members
    h_model.append("};\n\n")
    c_model.append("")


def generateView(data):
    """This creates a simple view."""

    view = data["view"]
    global h_include
    addInclude(h_include, view["type"])
    global h_view
    h_view.append("class %s : public %s" % (view["name"], view["type"]))
    h_view.append("{")
    h_view.append("	Q_OBJECT")
    h_view.append("public:")
    h_view.append("	%s(QWidget *parent=0);" % view["name"])
    # TODO: members

    global c_view
    c_view.append("%s::%s(QWidget *parent)" % (view["name"],view["name"]))
    c_view.append("\t: %s(parent)" % view["type"])
    # TODO: members
    c_view.append("{")
    # TODO: should this be configurable?
    c_view.append("	setAlternatingRowColors(true);")
    c_view.append("	verticalHeader()->hide();")
    c_view.append("	horizontalHeader()->setResizeMode(QHeaderView::Stretch);")
    c_view.append("	setTabKeyNavigation(false);")
    c_view.append("\tsetHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);")
    c_view.append("\tsetVerticalScrollMode(QAbstractItemView::ScrollPerPixel);")
    c_view.append("")
    if get(view, "sort", True):
        c_view.append("\tconnect(horizontalHeader(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)),")
        c_view.append("\t        SLOT(sortByColumn(int)) );")
        c_view.append("\tsetSortingEnabled(true);")
        c_view.append("\tsortByColumn(0, Qt::AscendingOrder);")
        c_view.append("\tsetEditTriggers(QAbstractItemView::AnyKeyPressed | QAbstractItemView::EditKeyPressed);")
        if data.has_key("dialog"):
            c_view.append("\tconnect(this, SIGNAL(doubleClicked(const QModelIndex &)), SLOT(slotEdit(const QModelIndex &)) );")
    c_view.append("}\n")

    if get(view, "delete") or get(view, "insert"):
        generateViewInsertDelete(data)
    if data.has_key("dialog"):
        generateViewSlotEdit(data)

    h_view.append("};\n")


def generateViewSlotEdit(data):
    """Adds code to handle the call of the editor dialog due after a
    double-click."""

    view = data["view"]
    dia = data["dialog"]
    model = data["model"]
    global h_view
    h_view.append("public slots:")
    h_view.append("\tvoid slotEdit(const QModelIndex &index);")

    global c_view
    c_view.append("void %s::slotEdit(const QModelIndex &index)" % view["name"])
    c_view.append("{")
    c_view.append("\tint row = index.row();")
    c_view.append("\t%s m = chars[row];" % data["name"])
    c_view.append("\tQDialog *dia = new %s(&m, this);" % dia["name"])
    c_view.append("\tif (dia->exec() == QDialog::Accepted) {")
    c_view.append("\t\tchars[row] = m;")
    c_view.append("\t\tQModelIndex left = model()->index(row, 0);")
    c_view.append("\t\tQModelIndex right = model()->index(row, COL_%s_LAST);" % model["name"].upper())
    c_view.append("\t\tdataChanged(left, right);")
    c_view.append("\t}")
    c_view.append("\tdelete dia;")
    c_view.append("}\n")


def generateViewInsertDelete(data):
    view = data["view"]
    dia = data["dialog"]
    cont = data["container"]
    global c_include
    addInclude(c_include, "QKeyEvent")

    global h_view
    h_view.append("public slots:")
    h_view.append("\tvirtual void keyPressEvent(QKeyEvent *event);")

    global c_view
    c_view.append("void %s::keyPressEvent(QKeyEvent *event)" % view["name"])
    c_view.append("{")
    c_view.append("\tswitch (event->key()) {")
    if get(view, "insert"):
        c_view.append("\tcase Qt::Key_Insert: {")
        c_view.append("\t\t%s m;" % name)
        # Initialize field. A simple memset() doesn't work because of possible QStrings
        for field in data["fields"]:
            typ = get(field, "type")
            if not typ:
                continue
            default_code = get(field, "default_code")
            default = get(field, "default")
            if default_code:
                default = default_code
            elif default:
                if typ == "QString":
                    default = "\"%s\"" % default
            elif typ == "bool":
                default = "false"
            elif typ == "QString":
                default = "QString::null";
            elif typ == "quint32":
                default = "0"
            else:
                raise "Unknown default for field type %s" % typ
            c_view.append("\t\tm.%s = %s;" % (field["name"], default) )
        c_view.append("\t\tQDialog *dia = new %s(&m, this);" % dia["name"])
        c_view.append("\t\tif (dia->exec() == QDialog::Accepted) {")
        c_view.append("\t\t\tint row = 0;")
        c_view.append("\t\t\tif (currentIndex().isValid())")
        c_view.append("\t\t\t\trow = currentIndex().row();")
        c_view.append("\t\t\tif (model()->insertRow(row))")
        c_view.append("\t\t\t\tsetCurrentIndex(model()->index(row+1, 0));");
        c_view.append("\t\t\t%s[row] = m;" % cont["name"])
        c_view.append("\t\t}")
        c_view.append("\t\tdelete dia;")
        c_view.append("\t\treturn;")
        c_view.append("\t\t}")
    if get(view, "delete"):
        c_view.append("\tcase Qt::Key_Delete:")
        c_view.append("\t\tif (!currentIndex().isValid())")
        c_view.append("\t\t\treturn;")
        # TODO: ask for confirmation?
        # TODO: check selection
        c_view.append("\t\tmodel()->removeRows(currentIndex().row(), 1);")
        c_view.append("\t\treturn;")
    c_view.append("\t}");
    c_view.append("\tQAbstractItemView::keyPressEvent(event);")
    c_view.append("}\n")


def editorForTyp(typ):
    """Given a Qt base type, returns a usable edit QWidget, and the
    names of the setter and getter functions."""

    if typ == "quint32":
        return ("QSpinBox", "setValue", "value")
    elif typ == "QString":
        return ("QLineEdit", "setText", "text")
    elif typ == "bool":
        return ("QCheckBox", "setChecked", "isChecked")
    return (None, None, None)


def generateDialog(data):
    """This hefty generator function generates a QDialog-derived class
    to out-of-table editing of data records."""

    dia = data["dialog"]
    global c_include

    title = get(dia, "head")
    if title:
        title = "tr(\"%s\")" % title
    else:
        title = "QString::null"

    global h_dialog;
    addInclude(h_include, "QDialog")
    h_dialog.append("class %s : public QDialog" % dia["name"])
    h_dialog.append("{")
    h_dialog.append("\tQ_OBJECT")
    h_dialog.append("public:")
    h_dialog.append("\t%s(%s *record, QWidget *parent=0, Qt::WindowFlags f=0);" % (dia["name"], data["name"]))
    h_dialog.append("\tvirtual void accept();")
    h_dialog.append("")
    h_dialog.append("\t%s *m;" % data["name"])

    global c_dialog
    c_dialog.append("%s::%s(%s *record, QWidget *parent, Qt::WindowFlags f)"
                    % (dia["name"], dia["name"], data["name"]))
    c_dialog.append("\t: QDialog(parent, f)")
    c_dialog.append("\t, m(record)")
    c_dialog.append("{")
    c_dialog.append("\tsetWindowTitle(%s);" % title)
    c_dialog.append("")

    c_layout = []
    c_get    = []
    c_set    = []
    for field in data["fields"]:
        if not (field.has_key("type") and field.has_key("name") and field.has_key("head")):
            continue
        if get(field, "readonly"):
            continue
        name = "edit%s" % field["name"].capitalize()
        typ = get(field, "dia_type", field["type"])
        (typ, setter, getter) = editorForTyp(typ)
        if typ == "":
            print "Unknown type '%s' for dialog" % field["type"]
            continue
        if typ[0] == "Q":
            addInclude(c_include, typ)
            addClassRef(typ)
        # TODO: allow include of local header?
        h_dialog.append("\t%s *%s;" % (typ, name) )

        c_dialog.append("\t%s = new %s(this);" % (name, typ))
        c_set.append("\t%s->%s(m->%s);" % (name, setter, field["name"]))
        label = get(field, "dia_label", get(field, "head"))
        if label:
            label = "tr(\"%s\"), " % label
        c_layout.append("\tformLayout->addRow(%s%s);" % (label, name))

        c_get.append("\tm->%s = %s->%s();" % (field["name"], name, getter))

    h_dialog.append("\tQDialogButtonBox *okCancel;");
    h_dialog.append("};\n")

    addInclude(c_include, "QFormLayout")
    c_dialog.append("")
    c_dialog.append("\tQFormLayout *formLayout = new QFormLayout;")
    c_dialog.extend(c_layout)
    c_dialog.append("")
    c_dialog.extend(c_set)
    c_dialog.append("")

    addClassRef("QDialogButtonBox")
    addInclude(c_include, "QDialogButtonBox")
    c_dialog.append("\tokCancel = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);")
    c_dialog.append("\tconnect(okCancel, SIGNAL(accepted()), this, SLOT(accept()));")
    c_dialog.append("\tconnect(okCancel, SIGNAL(rejected()), this, SLOT(reject()));")
    c_dialog.append("")
    c_dialog.append("\tQVBoxLayout *mainLayout = new QVBoxLayout(this);")
    c_dialog.append("\tmainLayout->addLayout(formLayout);")
    c_dialog.append("\tmainLayout->addWidget(okCancel);")
    c_dialog.append("\tsetLayout(mainLayout);")
    c_dialog.append("}\n")

    c_dialog.append("void CharacterDialog::accept()")
    c_dialog.append("{")
    c_dialog.extend(c_get)
    c_dialog.append("\tQDialog::accept();")
    c_dialog.append("}\n")



#
# Very simple option parsing
#
from optparse import OptionParser
parser = OptionParser()
parser.add_option("-d", "--dir", dest="destdir", default=".",
	help="destination directory", metavar="DIR")
parser.add_option("--decl",
                  action="store_true", dest="decl", default=False,
                  help="generate declaration (*.h)")
parser.add_option("--impl",
                  action="store_true", dest="impl", default=False,
                  help="generate implementation (*.cpp)")
(options, args) = parser.parse_args()
options.both = not options.decl and not options.impl
if len(args) != 1:
    print "Usage: %s -o <output> <filename>.yaml" % sys.argv[0]


#
# Load the .yaml file into a python structures
#
stream = open(args[0])
alldata = yaml.load(stream, Loader=yaml.CLoader)


#
# Loop over all entries and create arrays of string in various h_* /
# c_* files (for header & source code code segments)
#
for name in alldata:
    data = alldata[name]
    data["name"] = name
    generateStruct(data)
    generateContainer(data)
    if not data.has_key("code"):
        data["code"] = {}
    if data.has_key("model"):
        generateModel(data)
    if data.has_key("view"):
        generateView(data)
    if data.has_key("dialog"):
        generateDialog(data)
h_include.append("\n")
if h_classes: h_classes.append("\n")
if h_view: h_view.append("")

#print "\n".join(h_include)
#print "\n".join(h_classes)
#print "\n".join(h_dialog)
#print "\n".join(c_include)
#print "\n".join(c_dialog)
#sys.exit(1)

#
# Now write this into the two files
#
basename = os.path.splitext(os.path.basename(args[0]))[0]
if options.decl or options.both:
    f = open("%s/%s.h" % (options.destdir, basename), "w")
    f.write("#ifndef %s_H\n" % basename.upper())
    f.write("#define %s_H\n\n" % basename.upper())
    f.write("// automatically generated from %s\n\n" % args[0])
    f.write("\n".join(h_include))
    f.write("\n".join(h_classes))
    f.write("\n".join(h_struct))
    f.write("\n".join(h_container))
    f.write("\n".join(h_model))
    f.write("\n".join(h_view))
    f.write("\n".join(h_dialog))
    f.write("\n#endif\n")

if options.impl or options.both:
    f = open("%s/%s.cpp" % (options.destdir, basename), "w")
    f.write("#include \"%s.h\"\n" % basename)
    addInclude(c_include, "QHeaderView")
    f.write("\n".join(c_include))
    f.write("\n// automatically generated from %s\n\n" % args[0])
    f.write("\n".join(c_container))
    f.write("\n".join(c_col))
    if has_sort:
        f.write("static Qt::SortOrder sortOrder = Qt::AscendingOrder;\n\n")
    f.write("\n".join(c_model))
    f.write("\n".join(c_view))
    if c_dialog:
        f.write("\n\n")
        f.write("\n".join(c_dialog))
