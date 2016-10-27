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


  // 1 ===================================================================================
  // FUTURES and how to make them run async but then hit a point where they can sync up
  // 1 ===================================================================================
  println("== 1 ============================")

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
    Thread.sleep(Random.nextInt(500))
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
    Thread.sleep(Random.nextInt(500))
    println("milk : shutting down frothing system")
    s"frothed $milk"
  }

  def brew(coffee: GroundCoffee, heatedWater: Water): Future[Espresso] = Future {
    println("brew : happy brewing :)")
    Thread.sleep(Random.nextInt(500))
    println("brew : it's brewed!")
    "espresso"
  }

  def combine(espresso: Espresso, frothedMilk: FrothedMilk): Cappuccino = s"${espresso}-${frothedMilk} cappuccino"

  def makeACupAndWaitForItToFinish() =
  {
    // Let's make some coffee...
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

    // WAIT for the damned coffee to finish before you quit, silly Scala!
    // Sometimes you NEED synchronous behavior, like when determining when to quit a command line app.
    // NOTE that in any server scenario, blocking makes little to no sense.
    Await.ready(results,Duration.Inf)
  }

  // Uncomment to run this scrap
  // makeACupAndWaitForItToFinish()

  // NOTE that in the goofy scala world, this kinda finishes here
  // but the final logging may happen after the next block starts.
  // Apparently, deterministic behavior is only available via true functional chaining.


  // 2 ===================================================================================
  // What do you use a promise for?  Not sure yet...
  // not for synchronous guarantee it seems - this doesn't block app to finish...
  // AH, a future IS a promise.  This just splits them.  What do you do with it then?
  // 2 ===================================================================================
  println("== 2 ============================")

  // This takes up a thread and doesn't let go
  // I'm turning it off so subsequent scrap is not impacted...
  /*
  def makeACoffeeViaPromise() = Future {
    println("instant: start to make a slow instant coffee")
    Thread.sleep(10000)
    println("instant: done.  gross.")
  }
  val p = Promise[Unit]
  val f = makeACoffeeViaPromise()
  f.onComplete {
    case Success(t) =>  p.success(t)
    case Failure(t) => p.failure(t)
  }
  p.future
  */


  // 3 ===================================================================================
  // Container looping with futures
  // 3 ===================================================================================
  println("== 3 ============================")

  type Smoothie = String

  // Small list
  // val smoothies = List[Smoothie]( "mango", "blueberry", "strawberry", "banana" )

  // Big indexed "par" container
  // NOTE that it did not make much difference, it seems scala optimizes so that only VERY large collections are parallelized...?
  // val smoothies = List[Smoothie]( "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "mango", "blueberry", "strawberry", "banana" )
  //   .toIndexedSeq
  //   .par

  // Small indexed parallel-enable container
  val smoothies = List[Smoothie]( "mango", "blueberry", "strawberry", "banana" )
    .toIndexedSeq
    .par

  def makeSmoothie(smoothie: Smoothie) = Future {
    println(s"smoothie start: ${smoothie}")
    Thread.sleep(500)
    println(s"smoothie done : ${smoothie}")

    // A dirty little sleep here to get the logging out before something else jumps in!
    Thread.sleep(50)
  }

  // We can make a full set of smoothies fully async, if we want.
  // NOTE that this will suck up threads and therefore may interfere with subsequent code.
  // That's the Scala life!
  // var smoothiesMapResult = smoothies.map(smoothie => makeSmoothie(smoothie))

  // Filter and map the result as a parameter into a function
  // NOTE: This will not block
  // smoothies.filter(_ == "mango").map(smoothie => makeSmoothie(smoothie))

  // Map wrapped with a future, SO WE KNOW WHEN IT IS DONE.
  // How nice that I completely stumbled on this.
  // WORKS GREAT
  /*
  var smoothiesFuture = Future{
    smoothies.map(smoothie => makeSmoothie(smoothie))
  }
  smoothiesFuture.onComplete {
    case Success(t) => println("Smoothies are all done.")
    case Failure(t) => println(s"Smoothies didn't get done: ${t}")
  }
  */

  // Yes blocking is BAD BAD BAD, but you better do it
  // if you want to know when to exit your app.
  // Bah this doesn't actually block on the map internal operation.
  // It would take deeper blocking to get that done.
  // Not a good idea to spend effort on.
  // Await.result(smoothiesFuture, Duration.Inf)


  // 4 ===================================================================================
  // Need to handle future of futures completion
  // This is not working yet, needs deeper future tracking or maybe flatten?
  // 4 ===================================================================================
  println("== 4 ============================")

  type SmoothieOrder = String
  val orders = List[SmoothieOrder]( "jon", "cara", "stephen" )
    .toIndexedSeq
    .par

  def prepOrder(order: SmoothieOrder) = Future {

    // Each order is a full smoothie set
    var smoothiesFuture = Future{
      smoothies.map(smoothie => makeSmoothie(smoothie))
    }
    smoothiesFuture.onComplete {
      case Success(t) => println(s"${order} smoothies are all done.")
      case Failure(t) => println(s"Smoothies didn't get done: ${t}")
    }
  }

  val ordersFuture = Future{
    orders.map(order => prepOrder(order))
  }

  // How are we supposed to flatten this?
  ordersFuture.onComplete {
    case Success(t) => println("Orders are all done.")
    case Failure(t) => println(s"Orders didn't get done: ${t}")
  }
  /*
  val ordersFlatFuture = ordersFuture.map(Future.sequence(_))
  ordersFlatFuture.onComplete {
    case Success(t) => println("Orders are all done.")
    case Failure(t) => println(s"Orders didn't get done: ${t}")
  }
  */



  // HACK to let things finish
  Thread.sleep(20000)

}
