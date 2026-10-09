import gz.msgs.air_speed_pb2
import gz.msgs.altimeter_pb2
import gz.msgs.imu_pb2
import gz.msgs.fluid_pressure_pb2
import gz.msgs.double_pb2


from google.protobuf.message_factory import MessageFactory
from google.protobuf.descriptor_pool import Default


def get_gz_message_from_string(message):
    """
    Get a gazebo message class from a string.

    NOTE: This method only works if the generated module was imported in this file (message descriptors
    generated in runtime).
    """
    full_name = "gz.msgs." + message if not message.startswith("gz.msgs") else message
    pool = Default()
    msg_descr = pool.FindMessageTypeByName(full_name)
    return MessageFactory(pool).GetPrototype(msg_descr)
