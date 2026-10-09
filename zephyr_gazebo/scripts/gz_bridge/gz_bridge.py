import argparse

from google.protobuf import json_format

from gz.transport import Node

from gz_message_pb2 import GazeboMessage
from gz.msgs.double_pb2 import Double

import time
import socket


from topic_map import TopicMap
from gz_pb_messages import get_gz_message_from_string
from dt_helper import find_gazebo_drivers, read_string_property

LOCAL_IP = "127.0.0.1"
LOCAL_PORT = 9002

parser = argparse.ArgumentParser()

parser.add_argument("--dts-file")
parser.add_argument("--map-file", action="append")

args = parser.parse_args()

tcp_socket = socket.socket(family=socket.AF_INET, type=socket.SOCK_STREAM, proto=socket.IPPROTO_TCP)

curr_conn = None

class GazeboBridge():
    def __init__(self, topic_map):
        self.topic_map = topic_map
        self.node = Node()
        self.publishers = {}


    def register_driver_topics(self, dts_file):
        drivers = find_gazebo_drivers(dts_file)

        for compat, driver in drivers.items():
            topics = self.topic_map.get_topics_for_compat(compat)
            for topic in topics:
                topic_name = read_string_property(driver, topic["topic_property"])
                topic_msg_type = topic["messages"]
                topic_msg_proto = get_gz_message_from_string(topic_msg_type)
                topic_type = topic["type"]

                if (topic_type == "subscribe"):
                    print(f"Subscribing to '{topic_msg_proto.__name__}' messages on topic '{topic_name}'")

                    # Create a new callback for each topic so that "topic" name field can be set and used
                    # to differentiate messages
                    def msg_cb(msg):
                        wrapped = GazeboMessage()
                        wrapped.topic = topic_name
                        wrapped.payload.Pack(msg)

                        if(curr_conn != None):
                            curr_conn.sendall(wrapped.SerializeToString())
                        else:
                            print("Client not connected")

                    self.node.subscribe(topic_msg_proto, topic_name, msg_cb)

                if (topic_type == "publish"):
                    print(f"Advertising '{topic_msg_proto}' messages on topic '{topic_name}'")
                    self.publishers[topic_name] = self.node.advertise(topic_name, topic_msg_type)

    def register_app_topics(self):
        topics = self.topic_map.get_app_topics()
        for topic in topics:
            topic_name = topic["topic_name"]
            topic_msg_type = topic["messages"]
            topic_msg_proto = get_gz_message_from_string(topic_msg_type)
            topic_type = topic["type"]

            if (topic_type == "subscribe"):
                print(f"Subscribing to '{topic_msg_proto}' messages on topic '{topic_name}'")

                # Create a new callback for each topic so that "topic" name field can be set and used
                # to differentiate messages
                def msg_cb(msg):
                    print(f"Received message for topic: {topic_name}")
                    wrapped = GazeboMessage()
                    wrapped.topic = topic_name
                    wrapped.payload.Pack(msg)

                    if(curr_conn != None):
                        curr_conn.sendall(wrapped.SerializeToString())
                    else:
                        print("Client not connected")

                self.node.subscribe(topic_msg_proto, topic_name, msg_cb)

            if (topic_type == "publish"):
                print(f"Advertising '{topic_msg_proto.__name__}' on '{topic_name}'")
                self.publishers[topic_name] = self.node.advertise(topic_name, topic_msg_proto)


    def publish_msg(self, topic, msg):
        self.publishers[topic].publish(msg)


def run_bridge():
    global curr_conn
    # create a transport node
    node = Node()
    map = TopicMap()
    map.add_from_file("map_builtin.yaml")

    for file in args.map_file:
        map.add_from_file(file)

    gz_bridge = GazeboBridge(map)

    gz_bridge.register_driver_topics(args.dts_file)
    gz_bridge.register_app_topics()

    with socket.socket(family=socket.AF_INET, type=socket.SOCK_STREAM, proto=socket.IPPROTO_TCP) as tcp_socket:
        tcp_socket.bind((LOCAL_IP, LOCAL_PORT))
        tcp_socket.listen()
        while True:
            print("Waiting for connection from Zephyr")
            curr_conn, addr = tcp_socket.accept()

            print(f"{addr} connected to server")

            while True:
                # If possible, receive data from connection as well
                try:
                    data, addr = curr_conn.recvfrom(1024)
                    if (len(data) == 0):
                        print("Connection closed!")
                        curr_conn.close()
                        curr_conn = None
                        break

                    elif (len(data) > 0):
                        msg = GazeboMessage()
                        msg.ParseFromString(data)
                        nested_type = get_gz_message_from_string(msg.payload.TypeName())
                        nested = nested_type()
                        ret = msg.payload.Unpack(nested)
                        if (ret):
                            gz_bridge.publish_msg(msg.topic, nested)

                except ConnectionResetError:
                    break

if __name__ == "__main__":
    run_bridge()
