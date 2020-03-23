use rocket::Request;
use rocket_contrib::templates::Template;

use super::*;

#[catch(404)]
pub fn not_found(_req: &Request) -> Template {
    let context = Context {title: "404".to_string(), lang: "en".to_string()};
    Template::render("404", &context)
}
