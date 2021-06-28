package main

import (
	"bufio"
	"flag"
	"fmt"
	"log"
	"os"
	"strconv"
	"time"

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
	outputMB := flag.String("o", "service1", "Output Mailbox")
	serviceName := flag.String("n", "dataSource1", "Service name")
	tsFile := flag.String("t", "tsCal.csv", "timestamp file path")
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
		*outputMB, // name
		false,     // durable
		false,     // delete when unused
		false,     // exclusive
		false,     // no-wait
		nil,       // arguments
	)
	failOnError(err, "Failed to declare a queue")

	/**
	* Trigger function
	 */
	totSent := 0
	trigger := func() {
		totSent++
		fmt.Println(*serviceName, "triggered. Send message to", *outputMB, "req number", totSent, *scenarioName)
		body := fmt.Sprint(time.Now().UnixNano())
		err = ch.Publish(
			"",     // exchange
			q.Name, // routing key
			false,  // mandatory
			false,  // immediate
			amqp.Publishing{
				ContentType: "text/plain",
				Body:        []byte(body),
			})
		log.Printf(" [x] Sent %s", body)
	}

	/* open file and trigger when necessary */
	file, err := os.Open(*tsFile)
	if err != nil {
		log.Fatal(err)
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)
	prev := 0.

	// POSSIBILITY (not sure what would be recommended): add a goroutine listening to a channel onto which we put 1 value for each TS
	for scanner.Scan() {
		tsTrigger, _ := strconv.ParseFloat(scanner.Text(), 64)
		durSleep := tsTrigger*float64(time.Second) - prev

		time.Sleep(time.Duration(durSleep))
		trigger()
		//fmt.Println(scanner.Text(), " ", start.UnixNano())
		prev = tsTrigger * float64(time.Second)
	}

	failOnError(err, "Failed to publish a message")

}
