from schema import Schema, Or, And, Use
import yaml

driver_topic_schema = Schema({
        "compatible": str,
        "topic_property": str,
        "messages": Or([str], str),
        "type": And(str, Use(str.lower), lambda s: s in ["subscribe", "publish"])
    },
)

app_topic_schema = Schema({
        "topic_name": str,
        "messages": Or([str], str),
        "type": And(str, Use(str.lower), lambda s: s in ["subscribe", "publish"])
    },
)

map_schema = Schema({
    "topics": [
        Or(
            driver_topic_schema,
            app_topic_schema,
        ),
    ]
})

class TopicMap():
    def __init__(self):
        self.topics = []

    def add_from_file(self, file):
        with open(file) as f:
            data = yaml.safe_load(f)
            validated = map_schema.validate(data)
            self.topics.extend(validated["topics"])

    def get_driver_topics(self):
        return [t for t in self.topics if driver_topic_schema.is_valid(t)]

    def get_app_topics(self):
        return [t for t in self.topics if app_topic_schema.is_valid(t)]

    def get_topics_for_compat(self, compat):
        return [t for t in self.get_driver_topics() if t["compatible"] == compat]
