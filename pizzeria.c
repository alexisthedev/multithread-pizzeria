#include "pizzeria.h"

// Global Variables
unsigned int seed;
int total_income = 0;
int plain_pizzas_made = 0;
int special_pizzas_made = 0;
int cooks = N_COOK;
int ovens = N_OVEN;
int packers = N_PACKER;
int deliveras = N_DELI;
int total_time_spent = 0;
int max_order_time = 0;
int total_cooling_time = 0;
int max_cooling_time = 0;

// Mutexes
pthread_mutex_t PRINT_MUTEX;
pthread_mutex_t PIZZA_STAT_MUTEX;
pthread_mutex_t PREP_MUTEX;
pthread_mutex_t OVEN_MUTEX;
pthread_mutex_t PACK_MUTEX;
pthread_mutex_t DELI_MUTEX;
pthread_mutex_t TIME_MUTEX;

// Cond Vars
pthread_cond_t PREP_COND;
pthread_cond_t OVEN_COND;
pthread_cond_t PACK_COND;
pthread_cond_t DELI_COND;

void print_msg(char* msg, int oid) {
    /* This function is used as a shorthand
        for when a thread must print an output
        and should first lock the screen mutex.
    */

    int rc;
    rc = pthread_mutex_lock(&PRINT_MUTEX);
    if (rc != 0) {
        printf("[Order %d] Error: return code from pthread_mutex_lock() is %d.\n", oid, rc);
        pthread_exit(&rc);
    }
    printf("[Order %d] %s\n", oid, msg);
    rc = pthread_mutex_unlock(&PRINT_MUTEX);
    if (rc != 0) {
        printf("[Order %d] Error: return code from pthread_mutex_unlock() is %d.\n", oid, rc);
        pthread_exit(&rc);
    }
}

void init_mutex(pthread_mutex_t *M) {
    // Wrapper function to initialize mutexes and check for errors
    int rc;
    rc = pthread_mutex_init(M, NULL);
    if (rc != 0) {
   		printf("[Main] Error: return code from pthread_mutex_init() is %d\n", rc);
        exit(-1);
	}
}

void destroy_mutex(pthread_mutex_t *M) {
    // Wrapper function to destroy mutexes and check for errors
    int rc;
    rc = pthread_mutex_destroy(M);
    if (rc != 0) {
        printf("[Main] Error: return code from pthread_mutex_destroy() is %d\n", rc);
        exit(-1);
    }
}

void init_cond(pthread_cond_t *C) {
    // Wrapper function to initialize cond vars and check for errors
    int rc;
    rc = pthread_cond_init(C, NULL);
    if (rc != 0) {
   		printf("[Main] Error: return code from pthread_cond_init() is %d\n", rc);
        exit(-1);
	}
}

void destroy_cond(pthread_cond_t *C) {
    // Wrapper function to destroy cond vars and check for errors
    int rc;
    rc = pthread_cond_destroy(C);
    if (rc != 0) {
   		printf("[Main] Error: return code from pthread_cond_destroy() is %d\n", rc);
        exit(-1);
	}
}


void *makeOrder(void* t) {
    /* This function is the routine that
        handles a thread/order from end
        to end.
    */

    // Get order submission time
    struct timespec t_start;
    clock_gettime(CLOCK_REALTIME, &t_start);

    int oid = *(int*)t;
    int rc;
    unsigned int t_seed = seed + oid;
    int i;

    // Pizza assignment
    int pizza_num = N_ORD_LO + rand_r(&t_seed)%N_ORD_HI;
    int pizzas[pizza_num];
    for (i=0; i<pizza_num; i++) {
        // A pizza is plain with 60% chance, else it is special
        // 0: plain, 1: special
        pizzas[i] = (rand_r(&t_seed)%100 < 60) ? 0 : 1;
    }

    // Process order payment
    sleep(T_PAY_LO + rand_r(&t_seed)%T_PAY_HI);

    if (rand_r(&t_seed)%100 < 10) {
        // A payment fails with 10% chance

        // Print that the card was declined
        print_msg("Card declined, scrapping order!", oid);

        // Set rc to -1 for main() to know that this order
        // was scrapped because of payment failure
        rc = -1;
        pthread_exit(&rc);
    }
    print_msg("Cha-ching! Payment succesful.", oid);

    // Calculate order price and pizza%
    int plain = 0;
    int special = 0;
    for (i=0; i<pizza_num; i++) {
        plain += (pizzas[i]+1)%2;
        special += pizzas[i];
    }

    rc = pthread_mutex_lock(&PIZZA_STAT_MUTEX);
    if (rc != 0) {
        printf("[Order %d] Error: return code from pthread_mutex_lock() is %d.\n", oid, rc);
        pthread_exit(&rc);
    }

    plain_pizzas_made += plain;
    special_pizzas_made += special;
    total_income += plain*C_PLAIN + special*C_SPECIAL;

    rc = pthread_mutex_unlock(&PIZZA_STAT_MUTEX);
    if (rc != 0) {
        printf("[Order %d] Error: return code from pthread_mutex_unlock() is %d.\n", oid, rc);
        pthread_exit(&rc);
    }

    // Prepare order
    rc = pthread_mutex_lock(&PREP_MUTEX);
    if (rc != 0) {
        printf("[T%d] Error: return code from pthread_mutex_lock() is %d.\n", oid, rc);
        pthread_exit(&rc);
    }

    while (cooks < 1) {
        rc = pthread_cond_wait(&PREP_COND, &PREP_MUTEX);
        if (rc != 0) {
            printf("[Order %d] Error: return code from pthread_cond_wait() is %d.\n", oid, rc);
        }
    }
    cooks--;

    rc = pthread_mutex_unlock(&PREP_MUTEX);
    if (rc != 0) {
        printf("[Order %d] Error: return code from pthread_mutex_unlock() is %d.\n", oid, rc);
        pthread_exit(&rc);
    }

    // Cook prepares the order
    print_msg("Order now being prepared.", oid);
    sleep(T_PREP * pizza_num);

    rc = pthread_mutex_lock(&OVEN_MUTEX);
    if (rc != 0) {
        printf("[Order %d] Error: return code from pthread_mutex_lock() is %d.\n", oid, rc);
        pthread_exit(&rc);
    }

    while (ovens < pizza_num) {
        rc = pthread_cond_wait(&OVEN_COND, &OVEN_MUTEX);
        if (rc != 0) {
            printf("[Order %d] Error: return code from pthread_cond_wait() is %d.\n", oid, rc);
        }
    }
    ovens -= pizza_num;

    rc = pthread_mutex_unlock(&OVEN_MUTEX);
    if (rc != 0) {
        printf("[Order %d] Error: return code from pthread_mutex_unlock() is %d.\n", oid, rc);
        pthread_exit(&rc);
    }

    // Pizzas are in the oven, free the cook
    rc = pthread_mutex_lock(&PREP_MUTEX);
    if (rc != 0) {
        printf("[Order %d] Error: return code from pthread_mutex_lock() is %d.\n", oid, rc);
        pthread_exit(&rc);
    }

    cooks++;

    // Signal that the cook is done
    rc = pthread_cond_signal(&PREP_COND);
    if (rc != 0) {
        printf("[Order %d] Error: return code from pthread_cond_signal() is %d.\n", oid, rc);
    }
    rc = pthread_mutex_unlock(&PREP_MUTEX);
    if (rc != 0) {
        printf("[Order %d] Error: return code from pthread_mutex_unlock() is %d.\n", oid, rc);
        pthread_exit(&rc);
    }

    // Order is baking
    print_msg("Order is in the oven.", oid);
    sleep(T_BAKE);

    // Keep bake completion time
    struct timespec t_baked;
    clock_gettime(CLOCK_REALTIME, &t_baked);

    // Order is being packed
    rc = pthread_mutex_lock(&PACK_MUTEX);
    if (rc != 0) {
        printf("[Order %d] Error: return code from pthread_mutex_lock() is %d.\n", oid, rc);
        pthread_exit(&rc);
    }

    while (packers < 1) {
        rc = pthread_cond_wait(&PACK_COND, &PACK_MUTEX);
        if (rc != 0) {
            printf("[Order %d] Error: return code from pthread_cond_wait() is %d.\n", oid, rc);
        }
    }
    packers--;

    rc = pthread_mutex_unlock(&PACK_MUTEX);
    if (rc != 0) {
        printf("[Order %d] Error: return code from pthread_mutex_unlock() is %d.\n", oid, rc);
        pthread_exit(&rc);
    }

    print_msg("Order is being packed.", oid);
    sleep(T_PACK * pizza_num);

    // Keep ready for delivery time
    struct timespec t_ready;
    clock_gettime(CLOCK_REALTIME, &t_ready);

    // Print how much time elapsed from order
    // submission until it was ready for delivery.
    // We do not use the print_msg shorthand here
    // because we want a more custom output.
    rc = pthread_mutex_lock(&PRINT_MUTEX);
    if (rc != 0) {
        printf("[Order %d] Error: return code from pthread_mutex_lock() is %d.\n", oid, rc);
        pthread_exit(&rc);
    }

    printf("[Order %d] Order was ready in %ld minutes.\n", oid, (t_ready.tv_sec - t_start.tv_sec));

    rc = pthread_mutex_unlock(&PRINT_MUTEX);
    if (rc != 0) {
        printf("[Order %d] Error: return code from pthread_mutex_unlock() is %d.\n", oid, rc);
        pthread_exit(&rc);
    }

    // Free the ovens
    rc = pthread_mutex_lock(&OVEN_MUTEX);
    if (rc != 0) {
        printf("[Order %d] Error: return code from pthread_mutex_lock() is %d.\n", oid, rc);
        pthread_exit(&rc);
    }

    ovens += pizza_num;

    // Signal that the ovens are done
    rc = pthread_cond_signal(&OVEN_COND);
    if (rc != 0) {
        printf("[Order %d] Error: return code from pthread_cond_signal() is %d.\n", oid, rc);
    }
    rc = pthread_mutex_unlock(&OVEN_MUTEX);
    if (rc != 0) {
        printf("[Order %d] Error: return code from pthread_mutex_unlock() is %d.\n", oid, rc);
        pthread_exit(&rc);
    }

    // Free the packer
    rc = pthread_mutex_lock(&PACK_MUTEX);
    if (rc != 0) {
        printf("[Order %d] Error: return code from pthread_mutex_lock() is %d.\n", oid, rc);
        pthread_exit(&rc);
    }

    packers++;

    // Signal that the packer is done
    rc = pthread_cond_signal(&PACK_COND);
    if (rc != 0) {
        printf("[Order %d] Error: return code from pthread_cond_signal() is %d.\n", oid, rc);
    }
    rc = pthread_mutex_unlock(&PACK_MUTEX);
    if (rc != 0) {
        printf("[Order %d] Error: return code from pthread_mutex_unlock() is %d.\n", oid, rc);
        pthread_exit(&rc);
    }

    // Order delivery
    rc = pthread_mutex_lock(&DELI_MUTEX);
    if (rc != 0) {
        printf("[Order %d] Error: return code from pthread_mutex_lock() is %d.\n", oid, rc);
        pthread_exit(&rc);
    }

    while (deliveras < 1) {
        rc = pthread_cond_wait(&DELI_COND, &DELI_MUTEX);
        if (rc != 0) {
            printf("[Order %d] Error: return code from pthread_cond_wait() is %d.\n", oid, rc);
            pthread_exit(&rc);
        }
    }
    deliveras--;

    rc = pthread_mutex_unlock(&DELI_MUTEX);
    if (rc != 0) {
        printf("[Order %d] Error: return code from pthread_mutex_unlock() is %d.\n", oid, rc);
        pthread_exit(&rc);
    }

    // Calculate delivery time
    int delivery_time = T_DELI_LO + rand_r(&t_seed)%T_DELI_HI;

    // Deliver the order
    sleep(delivery_time);

    // Keep time of delivery
    struct timespec t_delivered;
    clock_gettime(CLOCK_REALTIME, &t_delivered);
    long int order_time =  t_delivered.tv_sec - t_start.tv_sec;

    // Print how much time elapsed from order
    // submission until it was delivered.
    // Again, we do not use the print_msg shorthand
    // here because we want a more custom output.
    rc = pthread_mutex_lock(&PRINT_MUTEX);
    if (rc != 0) {
        printf("[Order %d] Error: return code from pthread_mutex_lock() is %d.\n", oid, rc);
        pthread_exit(&rc);
    }

    printf("[Order %d] Order was delivered in %ld minutes.\n", oid, order_time);

    rc = pthread_mutex_unlock(&PRINT_MUTEX);
    if (rc != 0) {
        printf("[Order %d] Error: return code from pthread_mutex_unlock() is %d.\n", oid, rc);
        pthread_exit(&rc);
    }

    // Count order times
    // towards the time stats
    rc = pthread_mutex_lock(&TIME_MUTEX);
    if (rc != 0) {
        printf("[Order %d] Error: return code from pthread_mutex_lock() is %d.\n", oid, rc);
    }

    total_time_spent += order_time;
    max_order_time = (order_time > max_order_time) ? order_time : max_order_time;

    long int cooling_time = t_delivered.tv_sec - t_baked.tv_sec;
    total_cooling_time += cooling_time;
    max_cooling_time = (cooling_time > max_cooling_time) ? cooling_time : max_cooling_time;

    rc = pthread_mutex_unlock(&TIME_MUTEX);
    if (rc != 0) {
        printf("[Order %d] Error: return code from pthread_mutex_unlock() is %d.\n", oid, rc);
    }


    // Deliveras returns to store
    sleep(delivery_time);

    // Free the deliveras
    rc = pthread_mutex_lock(&DELI_MUTEX);
    if (rc != 0) {
        printf("[Order %d] Error: return code from pthread_mutex_lock() is %d.\n", oid, rc);
        pthread_exit(&rc);
    }

    deliveras++;

    rc = pthread_cond_signal(&DELI_COND);
    if (rc != 0) {
        printf("[Order %d] Error: return code from pthread_cond_signal() is %d.\n", oid, rc);
        pthread_exit(&rc);
    }
    rc = pthread_mutex_unlock(&DELI_MUTEX);
    if (rc != 0) {
        printf("[Order %d] Error: return code from pthread_mutex_unlock() is %d.\n", oid, rc);
        pthread_exit(&rc);
    }

    pthread_exit(t);
}


int main(int argc, char **argv) {
    printf("[Main] Welcome to Papa's Threaderia. Now taking orders!\n");

    // Argument parsing
    if (argc != 3) {
        printf("[Main] Error: expected 2 args, the number of customers and a seed!\n");
        exit(-1);
    }

    int customers;
    customers = atoi(argv[1]);
    seed = (unsigned int)atoi(argv[2]);
    if (customers < 0) {
        printf("[Main] Error: the number of customers should not be negative!\n");
        exit(-1);
    }
    if (seed < 0) {
        printf("[Main] Error: the seed should not be negative!\n");
        exit(-1);
    }

    // Mutex and Cond initialization
    int rc;
    init_mutex(&PRINT_MUTEX);
    init_mutex(&PIZZA_STAT_MUTEX);
    init_mutex(&PREP_MUTEX);
    init_mutex(&OVEN_MUTEX);
    init_mutex(&PACK_MUTEX);
    init_mutex(&DELI_MUTEX);
    init_mutex(&TIME_MUTEX);

    init_cond(&PREP_COND);
    init_cond(&OVEN_COND);
    init_cond(&PACK_COND);
    init_cond(&DELI_COND);

    // Order thread creation
    pthread_t *threads = malloc(customers * sizeof(pthread_t));
    int i;
    int ids[customers];
    for (i=0; i<customers; i++) {
        ids[i] = i+1;
        rc = pthread_create(&threads[i], NULL, makeOrder, &ids[i]);
        if (rc != 0) {
            printf("[Main] Error: return code from pthread_create(t%d) is %d.\n", ids[i], rc);
            exit(-1);
        }
        printf("[Main] Submitted order %d.\n", ids[i]);
        if (i < customers-1) sleep(T_ORD_LO + rand_r(&seed)%T_ORD_HI);
    }

    // Join threads / close out orders
    int failed = 0;
    void* status;
    for (i=0; i<customers; i++) {
        rc = pthread_join(threads[i], &status);
        if (rc != 0) {
            printf("[Main] Error: return code from pthread_join(t%d) is %d.\n", ids[i], rc);
            exit(-1);
        }

        // Do not print for orders that had their payment declined
        // or that were stopped due to an error.
        if (*(int*)status != ids[i]) {
            failed++;
            continue;
        }
        printf("[Main] Closed out order %d.\n", ids[i]);
    }

    printf("\n[Main] Done for the day! Papa's Threaderia stats from today's orders:\n");
    printf("Total Income: $%d\nPlain Pizzas Sold: %d\nSpecial Pizzas Sold: %d\n",
        total_income, plain_pizzas_made, special_pizzas_made);
    printf("Successful Orders#: %d\nFailed Orders#: %d\n", customers-failed, failed);
    printf("Avg. time until delivery: %dmin\nMax time until delivery: %dmin\n", total_time_spent/(customers-failed), max_order_time);
    printf("Avg. order cooling time: %dmin\nMax order cooling time: %dmin\n", total_cooling_time/(customers-failed), max_cooling_time);

    // Destroy mutexes and conds / close up shop
    destroy_mutex(&PRINT_MUTEX);
    destroy_mutex(&PIZZA_STAT_MUTEX);
    destroy_mutex(&PREP_MUTEX);
    destroy_mutex(&OVEN_MUTEX);
    destroy_mutex(&PACK_MUTEX);
    destroy_mutex(&DELI_MUTEX);
    destroy_mutex(&TIME_MUTEX);

    destroy_cond(&PREP_COND);
    destroy_cond(&OVEN_COND);
    destroy_cond(&PACK_COND);
    destroy_cond(&DELI_COND);

    free(threads);
    return 0;
}
