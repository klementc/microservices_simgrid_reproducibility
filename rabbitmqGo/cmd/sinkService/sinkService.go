package main

import (
	"flag"
	"log"

	/* for rmq access */
	"github.com/streadway/amqp"
)

func failOnError(err error, msg string) {
	if err != nil {
		log.Fatalf("%s: %s", msg, err)
	}
}

func main() {
	// CLI flags
	rmqHost := flag.String("r", "amqp://guest:guest@localhost:5672/", "RabbitMQ host")
	inputMB := flag.String("i", "sink", "Input Mailbox")
	serviceName := flag.String("n", "sinkService1", "Service name")
	scenarioName := flag.String("s", "unknown", "Name of the executed scenario")

	flag.Parse()
	/* END CLI flags*/

	conn, err := amqp.Dial(*rmqHost)
	failOnError(err, "Failed to connect to RabbitMQ")
	defer conn.Close()

	ch, err := conn.Channel()
	failOnError(err, "Failed to open a channel")
	defer ch.Close()

	q, err := ch.QueueDeclare(
		*inputMB, // name
		false,    // durable
		false,    // delete when unused
		false,    // exclusive
		false,    // no-wait
		nil,      // arguments
	)
	failOnError(err, "Failed to declare a queue")

	msgs, err := ch.Consume(
		q.Name, // queue
		"",     // consumer
		true,   // auto-ack
		false,  // exclusive
		false,  // no-local
		false,  // no-wait
		nil,    // args
	)
	failOnError(err, "Failed to register a consumer")

	forever := make(chan bool)

	go func() {
		for range /*d := */ msgs {
			log.Println(*serviceName, "received a message", *scenarioName)

		}
	}()

	log.Printf(" [*] Sink Waiting for messages. To exit press CTRL+C")
	<-forever
}
