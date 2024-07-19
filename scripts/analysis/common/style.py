import matplotlib
import matplotlib.pyplot as plt


def apply_style():
    matplotlib.rcParams["text.usetex"] = True
    matplotlib.rcParams["mathtext.fontset"] = "stix"
    matplotlib.rcParams["font.family"] = "STIXGeneral"
    matplotlib.pyplot.title(r"ABC123 vs $\mathrm{ABC123}^{123}$")


color_schema = {
    "green": "#798376",
    "darkgreen": "#484E46",
    "blue": "#41678B",
    "darkblue": "#2D4861",
    "orange": "#CB9471",
    "darkorange": "#965B37",
    "red": "#B65555",
    "mint": "#6AA56E",
    "grey": "#616161",
    "lightgrey": "#EEEEEEEE",
    "myblue": "#385F85",
    "myred": "#AA4848",
}

colors_idxes = list(color_schema)
