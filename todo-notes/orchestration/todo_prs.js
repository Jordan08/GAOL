export const meta = {
  name: 'todo-pr-texts',
  description: 'Write, in French, the pull request text of each corrected point (title and body files), then proofread it against the branch',
  phases: [
    { title: 'Draft', detail: 'one agent per point writes the title and the body' },
    { title: 'Proofread', detail: 'a second agent checks the facts against the branch and the French' },
  ],
}

const SCR = '/tmp/claude-1001/-home-jninin-Documents-WORK-DEV-GAOL-GAOL-V8/334ea319-d088-4702-9b10-e0628647c222/scratchpad'
const MAIN = '/home/jninin/Documents/WORK/DEV/GAOL/GAOL_V8'

const DRAFT_SCHEMA = {
  type: 'object',
  properties: {
    title: { type: 'string' },
    body_file: { type: 'string' },
    summary: { type: 'string', description: 'one sentence, in English, of what the pull request does' },
  },
  required: ['title', 'body_file'],
}

const PROOF_SCHEMA = {
  type: 'object',
  properties: {
    verdict: { type: 'string', enum: ['ok', 'corrected', 'problem'] },
    corrections: { type: 'string', description: 'what you changed in the files, or the problem you could not solve' },
  },
  required: ['verdict'],
}

const RULES = `The text is for the maintainer of GAOL v5 (Jordan Ninin, who reads French and English) and will be read on GitHub as the description of a pull request into the branch configure-clean. It is written in FRENCH (title and body), in correct French with its accents, a factual and precise tone (no marketing, no filler), Markdown that GitHub renders. Identifiers, file names, options, commands and code stay verbatim in code spans (\`...\`); the project is named "GAOL v5". NO line crediting Claude or Claude Code anywhere (no "Generated with", no robot emoji, no Co-Authored-By): the maintainer forbids it in every text published for him. No emoji at all.`

function draftPrompt(it) {
  return `You write the text of a pull request for GAOL v5, one of the points of TODO.md corrected in a branch. Read ${SCR}/HOUSE_RULES.md first (the maintainer's standing rules; you only write two files, you push nothing and open nothing).

${RULES}

THE POINT: n° ${it.id} of TODO.md, "${it.title}". Branch: ${it.branch} (worktree ${SCR}/wt/${it.slug}, do not modify it). Base of the pull request: configure-clean (origin/configure-clean is ${it.base || 'a2ca992'}).
${it.stackedOn && it.stackedOn.length ? `THIS BRANCH IS STACKED: it was made on top of the branch(es) of the point(s) ${it.stackedOn.join(', ')}, so its diff against configure-clean also holds their commits until those pull requests are merged.` : 'This branch is made directly from configure-clean.'}

WHERE THE FACTS ARE (read them; verify the report against the branch, do not just copy it): the author's report, the reviews and the CI results in ${it.report_file} (JSON); point ${it.todo || it.id} of TODO.md in the worktree (the diagnosis the maintainer wrote); the commits and the diff: \`git -C ${MAIN} log --stat origin/configure-clean..${it.branch}\` and \`git -C ${MAIN} diff origin/configure-clean...${it.branch}\`.

WHAT TO WRITE (${SCR}/pr/${it.file}.title: the title, one line, at most about 90 characters, no trailing period, in French, saying what the correction does; ${SCR}/pr/${it.file}.md: the body). The body explains well what was done, in this order, with these headings (omit a heading only when it has nothing to say):
## Point n° ${it.todo || it.id} du TODO.md — a sentence recalling what the point asks.
## Problème — what goes wrong, with a small example or the numbers of the report (inputs and wrong output), for whom.
## Cause — why (the code and the reasoning, briefly).
## Correction — what changed, by file and function; the design choices and the alternatives that were rejected and why; what deliberately was not changed.
## Tests — the new or extended tests (file, what they check), and how it was shown that they fail without the correction (the command and the result the report gives).
## Validation — the builds and compilers tested locally with their results (numbers of tests), and the continuous integration: ${it.ci ? 'the runs on the branch: ' + it.ci : 'a line "CI : {{CI}}" that the maintainer\'s assistant replaces by the links of the runs'}.
## Effets — what changes for the users (behaviour, API, ABI, installed files, documentation), or "aucun".
## Dépendances et recouvrements — "Aucune dépendance." or precisely which pull requests must be merged first and why, written with the placeholder {{PR:<id of the point>}} for the number of the pull request of another point (for instance {{PR:1}} for point 1: the assistant replaces the placeholders by the real numbers when it creates the pull requests); then the line "{{OVERLAPS}}" alone on its line (the assistant replaces it by the pull requests that touch the same regions, if any).
## Suivi — "TODO.md, ChangeLog et doc/differences.md ne sont pas modifiés ici : la pull request de synthèse {{PR:bookkeeping}} retire le point ${it.todo || it.id} du TODO et consigne le changement." and, if the manual (manual/v5/gaol.tex) was changed, that the PDF committed is regenerated once in that same pull request to avoid binary conflicts between pull requests.
## Questions ouvertes — the decisions left to the maintainer and what was noticed and not fixed (from the report's open_questions; omit if none).
Be complete but not long-winded: a reader who did not follow this work must understand the change and be able to review it from this text. Keep the code examples short. Do not invent anything: every number and claim comes from the report or from what you checked in the branch.

Write the two files, then return their names.`
}

function proofPrompt(it, draft) {
  return `You proofread the text of a pull request for GAOL v5 that another agent wrote from a branch: ${SCR}/pr/${it.file}.title and ${SCR}/pr/${it.file}.md (point n° ${it.todo || it.id}, "${it.title}", branch ${it.branch}, worktree ${SCR}/wt/${it.slug}). Read ${SCR}/HOUSE_RULES.md first.

${RULES}

CHECK, and CORRECT DIRECTLY IN THE FILES (Edit) what is wrong:
1. Every factual claim against the branch (\`git -C ${MAIN} log --stat origin/configure-clean..${it.branch}\`, the diff, the report ${it.report_file}, point ${it.todo || it.id} of TODO.md): file and function names, numbers, test names, the validation results, what is said to have changed. A claim you cannot verify is removed or reworded, never left.
2. The French: spelling, accents, grammar, agreement, typography (space before ; : ! ? and inside « »), technical vocabulary; natural French, not a translation of English (keep English terms that are the names of things: "flush-to-zero", "build", commit, test...). Consistent tense.
3. The Markdown: headings, code spans, lists render properly on GitHub; the placeholders {{PR:<id>}}, {{OVERLAPS}}, {{CI}} are left exactly as they are (the assistant replaces them); no credit line for Claude or Claude Code, no emoji.
4. Completeness for a reviewer who did not follow the work; nothing invented; the title is one line in French without a trailing period.
Return "ok" if nothing needed changing, "corrected" with the list of what you changed, "problem" if something is wrong that you cannot fix (say what).`
}

const items = (args && args.items) || []
log(`${items.length} pull request texts to write`)
const out = await pipeline(
  items,
  (it) => agent(draftPrompt(it), { label: `pr-draft:${it.id}`, phase: 'Draft', schema: DRAFT_SCHEMA }),
  (draft, it) => (draft ? agent(proofPrompt(it, draft), { label: `pr-proof:${it.id}`, phase: 'Proofread', schema: PROOF_SCHEMA }).then((p) => ({ id: it.id, draft, proof: p })) : { id: it.id, draft: null }),
)
return out.filter(Boolean)
