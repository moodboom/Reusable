package com.bitpost.scala.scrap

// original brew silliness from:
// http://danielwestheide.com/blog/2013/01/09/the-neophytes-guide-to-scala-part-8-welcome-to-the-future.html

import scala.concurrent.ExecutionContext.Implicits.global

// MDM These are essential for futures and promises
import scala.concurrent.{Future, Promise}
import scala.util.{Failure, Success, Try}

// MDM SYNCHRONOUS BEHAVIOR, don't act like you never need it
import scala.concurrent.Await
import scala.concurrent.duration.Duration

import scala.util.Random


/**
  * Created by m on 9/30/16.
  */

object Scrap extends App {

  // Some type aliases, just for getting more meaningful method signatures:
  type CoffeeBeans = String
  type GroundCoffee = String
  case class Water(temperature: Int)
  type Milk = String
  type FrothedMilk = String
  type Espresso = String
  type Cappuccino = String

  def grind(beans: CoffeeBeans): Future[GroundCoffee] = Future {
    println("grind: start...")
    Thread.sleep(Random.nextInt(3000))
    // if (beans == "baked beans") throw GrindingException("are you joking?")
    println("grind: finished...")
    s"ground coffee of $beans"
  }

  def heatWater(water: Water): Future[Water] = Future {
    println("heat :starting to heat the water now")
    Thread.sleep(Random.nextInt(3000))
    println("heat : hot, it's hot!")
    water.copy(temperature = 85)
  }

  def frothMilk(milk: Milk): Future[FrothedMilk] = Future {
    println("milk : frothing system engaged!")
    Thread.sleep(Random.nextInt(3000))
    println("milk : shutting down frothing system")
    s"frothed $milk"
  }

  def brew(coffee: GroundCoffee, heatedWater: Water): Future[Espresso] = Future {
    println("brew : happy brewing :)")
    Thread.sleep(Random.nextInt(3000))
    println("brew : it's brewed!")
    "espresso"
  }

  def combine(espresso: Espresso, frothedMilk: FrothedMilk): Cappuccino = s"${espresso}-${frothedMilk} cappuccino"

  def makeCoffee() = Future {
    println("espresso: starting...")
    val groundCoffee = grind("arabica beans")
    val heatedWater = heatWater(Water(20))
    val frothedMilk = frothMilk("soymilk")
    val results = for {
      ground <- groundCoffee
      water <- heatedWater
      foam <- frothedMilk
      espresso <- brew(ground, water)
    } yield combine(espresso, foam)

    results onFailure { case t => println(s"wtf ${t.getMessage}") }
    results onSuccess {
      case actualFinalResults =>
        println(s"espresso: done with ${actualFinalResults}")
    }

    // Sometimes you NEED synchronous behavior.
    val done = Await.ready(results,Duration.Inf)
  }

  // Sometimes you NEED synchronous behavior.
  // I do not like the fact that Scala acts like you don't.
  val futureCoffee = makeCoffee()
  val done = Await.ready(futureCoffee,Duration.Inf)

  // Still not sure what use a promise is...
  /*
  val p = Promise[Unit]
  val f = makeCoffee()
  f.onComplete {
    case Success(t) =>  p.success(t)
    case Failure(t) => p.failure(t)
  }

  p.future
  */

}
