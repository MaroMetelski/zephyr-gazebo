from devicetree import dtlib


def read_string_property(node, property):
    """Read a string property of the devicetree node"""
    prop = node.props.get(property, None)
    if prop:
        return prop.to_string()

    return None


def find_gazebo_drivers(dts_file):
    """
    Find all gazebo drivers under gz-bridge node in a devicetree file.

    Returns a dictionary of nodes with compatible string keys.
    """
    dt = dtlib.DT(dts_file)

    drivers = {}

    for node in dt.node_iter():
        compat = read_string_property(node, "compatible")
        if compat == "gz-bridge":
            for child in node.node_iter():
                child_compat = read_string_property(child, "compatible")
                # skip parent node
                if child_compat == compat:
                    continue
                # Index by compatible instead of nodelabel. Make it easy
                # to get the correct DRIVER_MAP entry
                drivers[child_compat] = child

    return drivers
