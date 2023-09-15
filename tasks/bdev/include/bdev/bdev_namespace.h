//
// Created by lukemartinlogan on 8/14/23.
//

#ifndef LABSTOR_TASKS_BDEV_INCLUDE_BDEV_BDEV_NAMESPACE_H_
#define LABSTOR_TASKS_BDEV_INCLUDE_BDEV_BDEV_NAMESPACE_H_

#include "bdev_tasks.h"
#include "bdev.h"
#include "labstor/labstor_namespace.h"

/** The set of methods in the admin task */
using ::hermes::bdev::Method;
using ::hermes::bdev::ConstructTask;
using ::hermes::bdev::DestructTask;
using ::hermes::bdev::AllocateTask;
using ::hermes::bdev::FreeTask;
using ::hermes::bdev::ReadTask;
using ::hermes::bdev::WriteTask;
using ::hermes::bdev::MonitorTask;
using ::hermes::bdev::UpdateCapacityTask;

/** Create admin requests */
using ::hermes::bdev::Client;

#endif  // LABSTOR_TASKS_BDEV_INCLUDE_BDEV_BDEV_NAMESPACE_H_
