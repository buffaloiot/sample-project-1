// Sensor Utilities

buildTopic = (base, target) => {
	topic = [base, target].join("/")
	topic = topic.replace(/^\//, "").replace(/\/[\/]+/, "/")
	return topic
}

module.exports = {
	buildTopic
}
