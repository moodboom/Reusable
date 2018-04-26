MDM CHECK THE YAML FIRST and set your rabbitmq config there, first thing.
I am not sure if you have to pre-create a "hello" exchange in rabbitmq.
(I did - I created "hello" exchange, "hello" queue, and routing from the  exchange to the queue).

# spring-amqp-kotlin
Spring AMQP app written in Kotlin

This app uses Spring AMQP to implement a simple producer/consumer pattern from the
first tutorial on the RabbitMQ website.

http://www.rabbitmq.com/tutorials/tutorial-one-spring-amqp.html

Instead of Spring/Java though, it uses Spring/Kotlin.  The logic has been
simplified slightly to remove the idea of it being part of a broader tutorial, but
otherwise is the same.  Launch the app as either a sender or receiver and it'll
act in that manner running for a time determined by a property.

To connect to external RabbitMQ brokers uncommend the properties in the
application.yml file or pass the parameters on the commandline.
