# Architecture of the Builder API backend

![Github actions](https://github.com/VeryBigThings/allied-builder-backend/workflows/Run+tests+%7C+deploy/badge.svg)


## Development setup
Prerequisites:
 * [docker-ce](https://docs.docker.com/engine/install/)
 * [docker-compose](https://docs.docker.com/compose/install/)

### Steps for running locally
* Run `make devstack` to build and run the development docker image
* Run `make shell` in a separate terminal window to connect to the running docker container
* While inside the container run `make dev` to run the server or `make test` to run the tests.
* API is available on http://localhost:4000
* To shut down you can use `make devstack-clean`

Env variables are located in `.env.development` file.

## Deployment
Deployment is done over Github Actions.
CI status can be followed on: https://github.com/VeryBigThings/allied-builder-backend/actions

* push to develop `develop` branch triggers deploy to `development` environment
* branches with `release-*` pattern trigger deploy to `staging` environment (example branch names: `release-1.0.10`)
* push to `master` branch triggers deploy to `production` environment

## Endpoints
* **Development**
API: https://develop-api.allied.build/
Desktop app: https://develop.allied.build/
Mobile app: https://develop.mobile.allied.build/

* **Staging**
API: https://staging-api.allied.build/
Desktop app: https://staging.allied.build/
Mobile app: https://staging.mobile.allied.build/

* **Production**
API: https://api.allied.build/
Desktop app: https://alliedbuildings.com/builder/index.html
Mobile app: https://mobile.allied.build/

## Workflow
### Before deploying to development
* Checkout fresh `develop`
* Create a feature branch whose name consist only of the short description of the change, written in a kebab-case (example: `remove-get-a-quote-mail`)
* Once done create a PR with branch `develop` as base branch
* Merge only after PR is approved by atleast one reviewer

### Before deploying to staging
* Checkout fresh `develop`
* Create a release branch in following format: `release-[release-number]` (example `release-1.0.12`)
* Increment application version in `mix.exs`. Version must correspond to the one in the release branch (example `1.0.12`).
* Update CHANGELOG.md with changes that are about to be released

### Before deploying to production
* Create a PR with `master` branch as base and `release-*` as head branch
* Merge only after PR is approved by atleast one reviewer

REST API docs are here:
* [staging API](https://api.allied.build/swagger/index.html)
* [production API](https://api.allied.build/swagger/index.html)

## 1) Responsibilities and general information

- persisting Leads, BuildingModels and Orders from the frontend to the local database
- pulls MBMACodes and Warnings from Google Sheets
- synchronizes Leads with Zoho
- validates primary building properties (width, height ...)
- calculates secondary building properties (wall surface, roof surface)
- calculates building costs
- checks discount validity and applies discounts to total cost
- manages Constants (building material prices, price adjustments, etc.)

Application code is implemented in **Elixir** programming language & **Phoenix** web framework.

Data stores used:
- **PostgreSQL** database is used to persist data (Leads, BuildingModels, Orders, MBMACodes...),
- **Redis** in-memory data store is used to schedule background jobs and cache often used data.

## 2) DB schema

Table schemas are defined [here](https://github.com/VeryBigThings/allied-builder-backend/tree/master/lib/allied_builder/schema)

Relevant models are clarified below:

### `BuildingModel`

```
id :: UUID
short_id :: string
data :: JSON
raw_data :: JSON
discount_code :: string
inserted_at :: timestamp
updated_at :: timestamp
```

* `short_id` - the short ID of the BuildingModel that's used to save a building over email.
* `data` - is a JSON column where the BuildingModel is saved. It consists of all building model fields such as width, height, length, etc.
* `raw_data` - building model raw data storage
* `discount_code` - code for building price discount

### `Lead`

```
id :: uuid
model_id :: uuid
zoho_id :: string
data :: json
zoho_sync_state :: string
zoho_sync_performed_at :: timestamp
zoho_tag_added_at :: timestamp
data_updated_at :: timestamp
details :: JSON
shipping :: JSON
billing :: JSON
payment_info :: JSON
inserted_at :: timestamp
updated_at :: timestamp
```

* `model_id` - the ID of the associated BuildingModel
* `zoho_id` - the ID of the Lead on Zoho, used to sync local copy of lead with remote copy.
* `data` - is a JSON column where the Lead is saved. It consists of all lead fields such as email, phone, etc...
* `zoho_sync_state` - status of last zoho lead sync
* `zoho_sync_performed_at` - time when last zoho lead sync was performed
* `zoho_tag_added_at` - time when zoho lead tagging was performed
* `data_updated_at` - time when lead was last updated after zoho syncing
* `details` - contact details. Broken down into:
  * `first_name :: string`
  * `last_name :: string`
  * `email :: string`
  * `phone :: string`
  * `timeframe :: string`
  * `business_type :: string`
* `shipping` - shipping address. Broken down into:
  * `company :: string`
  * `city :: string`
  * `zip_code :: string`
  * `country :: string`
  * `state :: string`
  * `street_address :: string`
  * `suite :: string`
* `billing` - billing address. Broken down into:
  * `company :: string`
  * `city :: string`
  * `zip_code :: string`
  * `country :: string`
  * `state :: string`
  * `street_address :: string`
  * `suite :: string`
* `payment_info` - payment information. Broken down into:
  * `cardholder_name :: string`
  * `discount_code :: string`

### `Discount`

```
id :: uuid
discount_code :: string

condition_type :: string
condition_field :: string
condition_value :: float

discount_type :: string
discount_field :: string
discount_value :: float

state :: string
expiry_date :: date
is_active :: boolean

inserted_at :: timestamp
updated_at :: timestamp
```

* `condition_type` - type of the condition, can have one of the following values:
  * "EQ" - equal
  * "NEQ" - not equal
  * "LT" - less than
  * "LTE" - less than or equal
  * "GT" - greater than
  * "GTE" - greater than or equal
* `condition_field` - determines the property of the building (including computed properties that condition is applied to, can also be: `no_condition` which means that discount always applies
* `condition_value` - determines the required value of the field
* `discount_type` - determines the type of the discount, can be: `percent` or `amount`
* `discount_field` - determines the field that the discount is applied to
* `discount_value` - determines the value of the discount
* `expiry_date` - determines how long the discount applies
* `state` - determines the USA state where the discount applies
* `is_active` - determines whether discount code is currently active
Discounts can be managed via the Admin UI:
https://api.allied.build/admin/discounts (production)
https://staging-api.allied.build/admin/discounts (staging)
https://develop-api.allied.build/admin/discounts (development)

### `Order`

```
id :: uuid
amount :: integer
submitted :: boolean

braintree_params :: json
braintree_response :: json
braintree_transaction_id ::string

lead_id :: uuid
discount_id :: uuid
building_model_id :: uuid
```

* `amount` - amount in cents
* `braintree_params` - params that are sent to braintree (saved for debugging purposes)
* `braintree_response` - response we get from the braintree (saved for debugging purposes)
* `lead_id` - uuid of the associated Lead
* `discount_id` - uuid of the associated Discount (if applies)
* `building_model_id` - uuid of the associated BuildingModel


### Factory

```
name :: string
street_address :: string
city :: string
zip_code :: string
state :: string
latitude :: float
longitude :: float
base_freight :: integer
price_per_mile :: float
```

Represents locations of steel factories where the building can be shipped from. Shipping calculation is done in ShippingCalculator

Factories can be managed via the Admin UI which is available here:
https://api.allied.build/admin/factories (production)
https://staging-api.allied.build/admin/factories (staging)
https://develop-api.allied.build/admin/factories (development)


### `MBMACode`

A local copy of the MBMA codes from Google spreadsheets. Synchronized every 15 minutes.

### `Constant`
| Property :: type | Description |
| :---------------------- | :---------- |
| `margin :: float` | A factor which represents the desired profit margin. |
| `gutters_per_foot_cost :: float` | Cost of gutters and downspouts per linear foot. |
| `wainscot_per_foot_cost :: float` | Cost for 3ft tall wainscot; applied by surface area. |
| `overhang_per_foot_cost :: float` | Cost for overhang by surface area. |
| `roof_paint_per_square_cost :: float` | Cost of roof paint when not Galvalaume. |
| `canopy_per_foot_cost :: float` | Canopy cost by surface area. |
| `leanto_per_foot_cost :: float` | Leanto cost by surface area. |
| `insulation_3_inch_cost :: float` | Cost for 3-inch thick insulation; applied on surface areas of wall and/or roof. |
| `insulation_4_inch_cost :: float` | Cost for 4-inch thick insulation; applied on surface areas of wall and/or roof. |
| `insulation_6_inch_cost :: float` | Cost for 6-inch thick insulation; applied on surface areas of wall and/or roof.
| `door3x7_cost :: float` | Opening cost. |
| `door4x7_cost :: float` | Opening cost. |
| `door6x7_cost :: float` | Opening cost. |
| `window3x3_cost :: float` | Opening cost. |
| `window6x3_cost :: float` | Opening cost. |
| `door3x7_insulated_wind_rated_cost :: float` | Opening cost. |
| `door4x7_insulated_wind_rated_cost :: float` | Opening cost. |
| `door6x7_insulated_wind_rated_cost :: float` | Opening cost. |
| `window3x3_insulated_wind_rated_cost :: float` | Opening cost. |
| `window6x3_insulated_wind_rated_cost :: float` | Opening cost. |
| `door3x7_insulated_cost :: float` | Opening cost. |
| `door4x7_insulated_cost :: float` | Opening cost. |
| `door6x7_insulated_cost :: float` | Opening cost. |
| `window3x3_insulated_cost :: float` | Opening cost. |
| `window6x3_insulated_cost :: float` | Opening cost. |
| `door3x7_frame_cost :: float` | Framed opening cost. |
| `door4x7_frame_cost :: float` | Framed opening cost. |
| `door6x7_frame_cost :: float` | Framed opening cost. |
| `window3x3_frame_cost :: float` | Framed opening cost. |
| `window6x3_frame_cost :: float` | Framed opening cost. |
| `rollup8x8_cost :: float` | Framed opening cost. |
| `rollup8x10_cost :: float` | Framed opening cost. |
| `rollup10x8_cost :: float` | Framed opening cost. |
| `rollup10x10_cost :: float` | Framed opening cost. |
| `rollup10x12_cost :: float` | Framed opening cost. |
| `rollup12x10_cost :: float` | Framed opening cost. |
| `rollup12x12_cost :: float` | Framed opening cost. |
| `rollup14x14_cost :: float` | Framed opening cost. |
| `rollup16x16_cost :: float` | Framed opening cost. |
| `price_adjustment_medium :: float` | Represents the increase/correction in steel prices quoted from Agora for buildings with 'medium' surface area. |
| `price_adjustment_large :: float` | Represents increase in steel prices/building quotes from Agora for buildings with 'large' surface area. |
| `xsmall_building_cost :: float` | Cost of xsmall buildings; used in both `wall_and_roof_cost` and `total_building_cost` formulas. |
| `small_building_cost :: float` | Cost of small buildings; used in both `wall_and_roof_cost` and `total_building_cost` formulas. |
| `small_to_medium_building_cost :: float` | Cost of small-to-medium buildings; used in both `wall_and_roof_cost` and `total_building_cost` formulas. |
| `medium_building_cost :: float` | Cost of medium buildings; used in both `wall_and_roof_cost` and `total_building_cost` formulas. |
| `large_building_cost :: float` | Cost of large buildings; used in both `wall_and_roof_cost` and `total_building_cost` formulas. |
| `xlarge_building_cost :: float` | Cost of xlarge buildings; used in both `wall_and_roof_cost` and `total_building_cost` formulas. |
| `xsmall_vector_anchor :: float` | Added xsmall buildings to 'anchor' them to the price vector/trend-line. |
| `small_vector_anchor :: float` | Added small buildings to 'anchor' them to the price vector/trend-line. |
| `small_to_medium_vector_anchor :: float` | Added small-to-medium buildings to 'anchor' them to the price vector/trend-line. |
| `medium_vector_anchor :: float` | Added medium buildings to 'anchor' them to the price vector/trend-line. |
| `large_vector_anchor :: float` | Added large buildings to 'anchor' them to the price vector/trend-line. |
| `xlarge_vector_anchor :: float` | Added xlarge buildings to 'anchor' them to the price vector/trend-line. |
| `sidewall_max_bay_space_length :: float` | Maximum length of a single bay. |
| `endwall_max_bay_space_length :: float` | Maximum width of a single bay. |
| `wainscot_height :: float` | Height of wainscot. |
| `default_roof_color :: string` | Default roof color. |
| `sqft_limit :: float` | Large building limit. |
| `wind_limit :: float` | High wind load limit. |
| `snow_limit :: float` | High snow load limit.  |
| `seismic_limit :: float` | High seismic load limit. |


Used to persist constants used in calculation of building properties and prices.

When managing these values, note that a new set of values is inserted every time one of them is changed (which means all old sets are saved for historic/debugging purposes).

Constants can be changed via the Admin UI which is available here:
https://api.allied.build/admin/constants (production)
https://staging-api.allied.build/admin/constants (staging)
https://develop-api.allied.build/admin/constants (development)

Username and password are set via [Heroku env vars](https://dashboard.heroku.com/apps/allied-builder-prod/settings)

The relevant env vars are `BASIC_AUTH_USERNAME` & `BASIC_AUTH_PASSWORD`.

To update constants associated with **freightCost**, see [Factories](https://github.com/VeryBigThings/allied-builder-backend#factory) section.


## 3) Main components

### [`BuildingConstructor`](/lib/allied_builder/building_constructor.ex)

Primary responsibility is converting user provided data (primary properties) to an actual `BuildingModel` object and computing all relevant secondary properties of the building.

There are two kinds of properties:
1) Primary (primitive) properties - properties that are set by the user through the UI,
2) Secondary (computed) properties - properties that are computed using primary properties.

Primary properties are

* `zipcode :: string`
* `width :: float`
* `length :: float`
* `height :: float`
* `pitch :: float` - pitch is usually expressed `P/12`, expressing how many inches of rise there are to 12 inches (one foot) of run - this value represents the `P` of `P/12`
* `color_roof :: string`
* `color_trim :: string`
* `color_walls :: string`
* `color_wainscot :: string`
* `gutters :: boolean` - determines whether the building has gutters or not
* `wainscot :: boolean` - determines whether the building has a wainscot or not
* `overhang_length :: float`
* `overhang_width :: float`
* `insulation_wall :: integer` - determines whether the walls are insulated
  * 0 -> no insulation
  * 1 -> 2 inch insulation
  * 2 -> 4 inch insulation
  * 3 -> 6 inch insulation
* `insulation_roof :: integer` - determines whether the roof is insulated
  * 0 -> no insulation
  * 1 -> 2 inch insulation
  * 2 -> 4 inch insulation
  * 3 -> 6 inch insulation
* `num_front_bays :: integer` - number of bays along the front sidewall. A **bay**, by definition, is **the distance between frames**. In the builder, you can imagine the frames beginning and ending at the columns that you see when viewing the interior shell of the model.
* `num_back_bays :: integer` - number of bays along the back sidewall
* `num_front_bays_with_openings :: integer`
* `num_back_bays_with_openings :: integer`
* `openings :: [string]` - a list of openings in form of `["window6x3", "rollup12x10", "door4x7"]`
* `bay_space_length :: float` - Length of a single bay
* `bay_space_width :: float` - Width of a single bay

Secondary properties are:

| Property :: type | Expression  | Description |
| :---------------------- | :------ | :---------- |
| `sqft :: float` | `width * length` | Total square footage of building. |
| `roof_run :: float` | `width / 2` | Reference [here](https://www.omnicalculator.com/construction/roof-pitch). |
| `roof_rise :: float` | `roof_run * pitch / 12` | Reference [here](https://www.omnicalculator.com/construction/roof-pitch). |
| `gable_surface :: float` | `width * roof_rise` | Reference [here](https://www.spikevm.com/how-to/gable-sq-ft.php). |
| `wall_surface :: float` | `(width + length) * height + gable_surface` | Equal surface of wall. |
| `roof_surface :: float` | `width * length / cos(atan2(roof_rise, roof_run))` | Reference [here](https://www.omnicalculator.com/construction/roofing). |
| `gutters_length :: float` | `2 * length + 2 * height * (num_front_bays + num_back_bays + 1)` | Determines length of gutters, if selected. |
| `overhang_surface :: float` | `overhang_width * width * 2 + overhang_length * length * 2 + overhang_width * overhang_length * 4` | Determines total surface area of overhangs, if selected. |
| `wainscot_surface :: float` | `width * wainscot_height * 2 + length * wainscot_height * 2` | Determines total surface area of wainscot, if selected. |
| `total_surface :: float` | `wall_surface + roof_surface + overhang_surface` | Combined, total surface area: wall + roof + gables. |
| `total_doors_number :: integer` | `count of door3x7, door4x7, door6x7`| Total count of doors based on openings. |
| `total_rollups_number :: integer` | `count of rollup8x8, rollup8x10, rollup10x8, rollup10x10, rollup10x12, rollup12x10, rollup12x12, rollup14x14, rollup16x16`| Total count of rollups based on openings. |
| `total_windows_number :: integer` |  `count of window3x3, window6x3`| Total count of windows based on openings. |


### [`BuildingCostCalculator`](/lib/allied_builder/building_cost_calculator.ex)

Calculates the cost of the building taking into account primary and secondary building properties.

The cost is calculated per logical segments. The segments are as follows:

| Property :: type | Expression  | Description |
| :---------------------- | :------ | :---------- |
| `wall_and_roof_cost :: float` | `total_surface * building_cost_factor + vector_anchor_factor` | Proportional to `total_surface`, modified by size modifiers (`xsmall_building_cost` & `xsmall_vector_anchor`). |
| `roof_paint_cost :: float` | `roof_surface * roof_paint_per_square_cost` | If roof color not Galvalaume, `roof_paint_cost` applied to surface area of roof. |
| `gutter_cost :: float` | `gutters_length * gutters_per_foot_cost` | Proportional to `gutters_length`, modified by `gutters_per_foot_cost`. |
| `wainscot_cost :: float` | `wainscot_surface * wainscot_per_foot_cost` | `wainscot_per_foot_cost` applied to surface area of 3-ft high wainscot. |
| `overhang_cost :: float` | `overhang_surface * overhang_per_foot_cost` | `overhang_per_foot_cost` applied to surface area of overhang, if any. |
| `wall_insulation_cost :: float` | `wall_surface * insulation_{thickness}_inch_cost` | Variable for wall insulation applied to surface area of walls, including gables. |
| `roof_insulation_cost :: float` | `roof_surface * insulation_{thickness}_inch_cost` | Variable for roof insulation applied to surface area of roof, excluding overhangs. |
| `insulation_cost :: map` | `wall_insulation_cost + roof_insulation_cost` | Sum of wall and roof insulation. As a map it is broken down into: `wall_insulation_cost` and `roof_insulation_cost`. |
| `openings_cost :: map` |  | Sum of the cost of each individual opening - every opening has its own price in the `Constant`s table. |
| `portal_frame_cost :: float` | `(sidewall_max_bay_space_length + 2 * height) * 14.5` | If all `front` bays have openings and/or all `back` bays have openings, apply formula for every case. |
| `base_building_cost :: float` | `wall_and_roof_cost + roof_paint_cost + gutter_cost + wainscot_cost + overhang_cost + portal_frame_cost + wall_insulation_cost + roof_insulation_cost + openings_cost` | The sum of all the previous "cost items" of the the building. |
| `total_building_cost :: float` |  | Taking the product of `base_building_cost`, concerns associated with local building codes (snow and wind) are added along with freight and profit margin. |
| `discount_amount :: float` | | Discount applied to `total_building_cost`. It is fetched from the `Discount` schema and can be percentage or amount. |
| `total_building_cost_w_discount ::float ` | `total_building_cost - discount_amount` | Final cost with applied discount. |
| `deposit_amount :: float` | `total_building_cost_w_discount / 10` | Initial deposit for building. |
| `shipping_cost :: float` | `base_freight + price_per_mile * distance` | Details [here](#shippingcostcalculator). |



### [`DiscountApplier`](/lib/allied_builder/discount_applier.ex)

Applies discount to the final cost, taking into account the building (constructed by `BuildingConstructor`) and the cost of the building (calculated by the `BuildingCostCalculator`).

Given a discount code, it first check whether the preconditions are satisfied, and applies the discount value if they are.

For example, given preconditions such as:

```
condition_field: "wall_surface",
condition_type: "GT",
condition_value: 1500
```

The discount is going to be applied only if the building has more than 1500 sqft of walls.

The effect of the discount is encoded in the "effect" fields:

```
discount_field: "insulation_cost",
discount_type: "PERCENTAGE",
discount_value: 25
```

... meaning that the 25% discount is going to be applied to `insulation_cost`.


### [`WarningService`](/lib/allied_builder/service/warning_service.ex)

Turns MBMACodes into warnings (based on user-provided zipcode).

Given a zipcode, it fetches the relevant `MBMACode` from the database and parses all the columns to see if any warnings apply for the given zipcode.

The rules are [here] (TODO: PROVIDE LINK TO GOOGLE DOC)

### [`ZohoIntegration`](/lib/allied_builder/integration/zoho_integration.ex)

The module responsible for syncing local copies of `Lead`s with Zoho using the **synchronization algorithm** defined below:

When a new lead is posted, or an existing lead is updated, it starts a 1 minute countdown timer before sending a `Lead` to Zoho. If no other updates are sent in that time window, the create/update action is performed. If an update arrives in the meantime, the countdown timer is rest.

This is done so we can avoid Zoho's rate limit which would happen because a lead is created updated on every `focusout` browser event.

Also responsible for preparing Lead data for Zoho.

### [`BraintreeIntegration`](/lib/allied_builder/integration/braintree_integration.ex)

Failry straigtforward. Once an new order is submitted, it calculates to price with the discount and submits a new sale to Braintree.

#### Declined Payments

When a payment is declined for whatever reason, a specific error code is printed with the decline message for troubleshooting.  These error codes correspond to the list outlined by Braintree, which can be [found here][2].

### [`MailingService`](/lib/allied_builder/service/mailing_service.ex)

Responsible for constructing and sending emails. There are two kinds of emails: `Customer` and `Internal`. `Customer` emails are emails sent to the customer, and `Internal` emails are sent to the Allied team.

Each function of the module is responsible for sending one kind of email:

Customer facing emails:
* `send_building_link` - sends the link to the building (on builder) to the user.
* `send_review_confirmation` - sends the email informing user the his building model needs to be reviewd by AlliedBuildings.
* `send_checkout_receipt` - send the receipt after a successful order submission and Braintree sale

Internal emails:
* `send_building_link_requested_notification` - send email to Allied team that someone requested a Building link
* `send_modal_submitted_notification` - send email to Allied team that someone submitted "further review required modal"
* `send_checkout_performed_notification` - send email to Allied that the customer performed a checkout
* `send_constants_changed_notification` - send notification about constant changes

#### Email Templates

HTML and text copy for emails can be found at [`allied-builder-backend/lib/allied_builder_web/templates/email/`](https://github.com/VeryBigThings/allied-builder-backend/tree/master/lib/allied_builder_web/templates/email).

### [`ShippingCostCalculator`](/lib/allied_builder/shipping_cost_calculator.ex)

Responsible for calculating distance between the customer's location (based on provided customer's zip code) and each of the available
steel factories. Based on the distance, it calculates the approximate cost of shipping. It takes into consideration the
`base_freight` and `price_per_mile` attributes available under each factory. After calculating the approximate cost, it
picks the factory with the cheapest shipping.

Actual formula for calculating the cost is `shipping_cost = base_freight + price_per_mile * distance`


## 4) Glossary

### Bay
A bay, by definition, is the distance between frames. In the builder, you can imagine the frames beginning and ending at the columns that you see when viewing the interior shell of the model.

In the screenshot below, there are four different bays marked out as an example. Although our pricing formula only takes into account the bays along the length (sidewalls) of the model, to be clear.
![Screen Shot 2019-09-13 at 1 49 05 PM](https://user-images.githubusercontent.com/12387497/64883755-1b843680-d62e-11e9-85b2-c9fff751a98d.png)


----------------

# ABOUT US

[<img src="https://github.com/hkdeven/AlliedBuildings.com/blob/master/ASB_Logo_Black_Horizontal%202.png?raw=true" align="right"/>][1]

[Allied][1] is a global leader in steel construction, developing solutions for every industry. From aviation to warehouses and everything in between, trust [Allied][1] to be with you all the way. The names and logos for Allied are trademarks of [Allied Steel Buildings, Inc.][1]

We love open source. :octocat: :heart:

[1]: http://alliedbuildings.com/builder
[2]: https://articles.braintreepayments.com/control-panel/transactions/declines#authorization-decline-codes
